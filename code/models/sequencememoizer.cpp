// Implemented from Gausthaus et. al 2010 (Lossless compression,...)
// Implementation in libPlump isn't of much help, because it's much
// more general (different parameter update flavors, undo+redo, fancy
// node allocation schemes). Re-implemented paper's description for
// simplicity.
#include <array>
#include <algorithm>
#include <functional>
#include <numeric>
#include <iostream>
#include <memory>
#include <utility>

#include <vector>
#include "data_types.hpp"
#include "model_ctx.hpp"
#include "model_mem.hpp"

double get_init_discount(std::size_t depth) {
    // Values from footnote 2
    static constexpr std::array<double, 11> g_discount_ar{0.05, 0.7, 0.8, 0.82, 0.84, 0.88, 0.91, 0.92, 0.93, 0.94, 0.95};
    // Clamp at end of discount ar
    auto idx = std::min(depth, g_discount_ar.size() - 1);
    return g_discount_ar[idx];
}
template <std::size_t num_children>
struct SMData {
    using count_t = std::uint8_t;
    using ProbAr = std::array<double, num_children>;
    using Ptr_t = std::uint32_t;
    std::array<count_t, num_children> m_cs{};
    std::array<count_t, num_children> m_ts{};
    std::size_t m_ctot{};
    std::size_t m_ttot{};
    double m_d{};
    std::array<Ptr_t, num_children> children{};
    std::size_t depth{};
    ProbAr weight_probs(ProbAr const &parent_probs) const {
        // See Eq1.
        if (!m_ctot) {
            return parent_probs;
        }
        ProbAr tmp;
        for (int i =0; i < tmp.size(); ++i) {
            tmp[i] = (m_cs[i] - m_d*m_ts[i] + m_d*m_ttot*parent_probs[i])/m_ctot;
        }
        return tmp;

    }
    ProbAr get_prior() const {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), 1.0/num_children);
        return ret;
    }
    bool update_counts(idx_t sym, ProbAr const& parent_probs) {
        // Unconditionally increment C
        m_cs[sym] += 1;
        m_ctot += 1;
        auto const RESCALE_THRESHOLD = std::numeric_limits<uint8_t>::max();
        if (m_cs[sym] == RESCALE_THRESHOLD) {
            std::cout << "RESCALE" << std::endl;
            auto new_total = 0;
            std::transform(m_cs.begin(), m_cs.end(), m_cs.begin(),
                           [this, &new_total](auto const &el) {
                               // Divide by 2, round up
                               auto nc =(el >> 1) + (el & 1);
                               new_total += nc;
                               return nc;
                           });
            m_ctot = new_total;
        }

        // Flip T, see old value. In Chinese Restaurant Parlance: we didn't make a new table
        auto old_t = std::exchange(m_ts[sym], 1);
        m_ttot += !old_t;
        return old_t;
    }
    SMData(std::size_t depth_) :depth{depth_}, m_d{get_init_discount(depth_)} {
        for (int i = 0 ; i < num_children; ++i) {
            assert(m_cs[i] == 0);
            assert(m_ts[i] == 0);
            assert(children[i] ==0);
        }
    }
};
template <std::size_t num_children>
class AmortSM {
    std::vector<SMData<num_children>> m_vec;
    std::size_t m_root{0};
    std::size_t m_depth;
    using ProbAr = std::array<double, num_children>;
    struct IdxAndProb {
        typename SMData<num_children>::Ptr_t idx;
        ProbAr prob;
    };
    std::vector<IdxAndProb> m_prealloc;
    auto get_deepest_prob(IdxContext const &ctx) const {
        auto idx = 0;
        auto ret= m_vec[0].weight_probs(m_vec[0].get_prior());
        for (IdxContext c = ctx; c; c.pop()) {
            auto const child_idx = m_vec[idx].children[c.back()];
            // Leave if can't make child
            if (!child_idx) {
                break;
            }
            ret = m_vec[child_idx].weight_probs(ret);
            idx=child_idx;

        }
        return ret;
    }
    auto get_prob_path(IdxContext const &ctx) {
        std::vector<IdxAndProb> cpps;
        cpps.reserve(m_depth+1);
        cpps.push_back(IdxAndProb{.idx=0, .prob=m_vec[0].get_prior()});
        for (IdxContext c = ctx; c; c.pop()) {
            // Note we don't form references to m_vec[idx] because we're resizing m_vec.
            auto const parent_idx = cpps.back().idx;
            auto child_idx = m_vec[parent_idx].children[c.back()];
            // Make child if needed:
            if (!child_idx) {
                m_vec[parent_idx].children[c.back()] = m_vec.size();
                child_idx = m_vec.size();
                m_vec.emplace_back(m_vec[parent_idx].depth+1);
            }
            // Push
            cpps.push_back(IdxAndProb{.idx=child_idx,
                                      .prob=m_vec[child_idx].weight_probs(cpps.back().prob)});
        }
        return cpps;
    }
public:
    auto size() const {
        return m_vec.size();
    }
    AmortSM(std::size_t depth) :m_depth{depth} {
        m_vec.reserve(1<<15);
        m_vec.emplace_back(0);
        m_prealloc.reserve(m_depth+1);
    }
    ProbAr get_probs(IdxContext const &ctx) const {
        return get_deepest_prob(ctx);
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        // idx, prob, pair.
        auto cpps = get_prob_path(ctx);
        // Work backwards, updating:
        // get_prob_path(ctx);
        for (auto itr = cpps.crbegin(); itr != cpps.crend(); ++itr) {
            if (m_vec[itr->idx].update_counts(sym, itr->prob)) {
                break;
            }
        }
    }
};


template <std::size_t num_children>
class SMNode {
    using ProbAr = std::array<double, num_children>;
    // using count_t = std::size_t;
    using count_t = std::uint8_t;
    std::array<std::unique_ptr<SMNode>, num_children> m_children{};
    std::array<count_t, num_children> m_cs{};
    std::array<count_t, num_children> m_ts{};
    std::size_t m_ctot{};
    std::size_t m_ttot{};
    std::size_t m_depth{};
    double m_d{};
    std::unique_ptr<SMNode> make_child() {
        return std::make_unique<SMNode>(m_depth+1);
    }
    SMNode& get_child(idx_t idx) {
        if (!m_children[idx]) {
            m_children[idx] = make_child();
        }
        return *m_children[idx];
    }
    SMNode const& get_child(idx_t idx) const {
        assert(m_children[idx]);
        return *m_children[idx];
    }

    ProbAr weight_probs(ProbAr const &parent_probs) const {
        // See Eq1.
        if (!m_ctot) {
            return parent_probs;
        }
        ProbAr tmp;
        for (int i =0; i < tmp.size(); ++i) {
            tmp[i] = (m_cs[i] - m_d*m_ts[i] + m_d*m_ttot*parent_probs[i])/m_ctot;
        }
        return tmp;

    }
    ProbAr get_prior() const {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), 1.0/num_children);
        return ret;
    }
    ProbAr get_probs_rec(IdxContext const &ctx, ProbAr const &parent_probs) const {
        // ! We don't need to because new children return parent probs, so fall through
        if (ctx && m_children[ctx.back()]) {
            return get_child(ctx.back()).get_probs_rec(ctx.popped(), weight_probs(parent_probs));
        }
        // This captures
        return weight_probs(parent_probs);
    }
    bool update_counts(idx_t sym, ProbAr const& parent_probs) {
        // Unconditionally increment C
        m_cs[sym] += 1;
        m_ctot += 1;
        auto const RESCALE_THRESHOLD = std::numeric_limits<uint8_t>::max();
        if (m_cs[sym] == RESCALE_THRESHOLD) {
            std::cout << "RESCALE" << std::endl;
            auto new_total = 0;
            std::transform(m_cs.begin(), m_cs.end(), m_cs.begin(),
                           [this, &new_total](auto const &el) {
                               // Divide by 2, round up
                               auto nc =(el >> 1) + (el & 1);
                               new_total += nc;
                               return nc;
                           });
            m_ctot = new_total;
        }

        // Flip T, see old value. In Chinese Restaurant Parlance: we didn't make a new table
        auto old_t = std::exchange(m_ts[sym], 1);
        m_ttot += !old_t;
        return old_t;
    }
    bool learn_down(IdxContext const &ctx, idx_t sym, ProbAr const &parent_probs) {
        // Traverse
        if (ctx) {
            auto broke = get_child(ctx.back()).learn_down(ctx.popped(), sym, weight_probs(parent_probs));
            if (broke) return broke;
        }
        return update_counts(sym, parent_probs);
    }
public:
    static inline std::size_t num_created;
    SMNode(std::size_t depth) :m_depth{depth}, m_d{get_init_discount(depth)} {++num_created;}
    ProbAr get_probs(IdxContext const &ctx) const {
        return get_probs_rec(ctx, get_prior());
    }

    void learn(IdxContext const& ctx, idx_t sym) {
        learn_down(ctx, sym, get_prior());
    }
};


template <typename AlphabetT>
class SequenceMemoizer {
public:
    using Alphabet = AlphabetT;
private:
    using sym_t = typename Alphabet::sym_t;
    using count_t = std::size_t;
    MemoryDeque<idx_t> m_past_idxs;
    using Node = SMNode<Alphabet::size>;
    SMNode<Alphabet::size> m_root;
public:
    Footprint footprint() const {
        return {.num_nodes = Node::num_created,
                .node_size = sizeof(Node)};
    }
    auto get_probs() const {
        return m_root.get_probs(m_past_idxs.view());
    }
    SequenceMemoizer (std::size_t depth): m_past_idxs{depth}, m_root{0} {
        Node::num_created=0;
    }
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_root.learn(view, idx);
        m_past_idxs.push_back(idx);
    }
};
template <typename AlphabetT>
class SequenceMemoizerAmort {
public:
    using Alphabet = AlphabetT;
private:
    using count_t = std::size_t;
    using sym_t = typename Alphabet::sym_t;
    MemoryDeque<idx_t> m_past_idxs;
    AmortSM<Alphabet::size> m_sm;
public:
    Footprint footprint() const {
        return {.num_nodes = m_sm.size(),
                .node_size = sizeof(SMData<Alphabet::size>)};
    }
    auto get_probs() const {
        return m_sm.get_probs(m_past_idxs.view());
    }
    SequenceMemoizerAmort (std::size_t depth): m_past_idxs{depth}, m_sm{depth} {}
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_sm.learn(view, idx);
        m_past_idxs.push_back(idx);
    }
};
