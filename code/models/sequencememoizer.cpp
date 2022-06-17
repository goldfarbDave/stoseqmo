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
class SMNode {
    using idx_t = size_t;
    using IdxContext = Context<idx_t>;
    using ProbAr = std::array<double, num_children>;
    using count_t = std::size_t;
    // using count_t = std::size_t;
    std::array<std::unique_ptr<SMNode>, num_children> m_children{};
    std::array<count_t, num_children> m_cs{};
    std::array<count_t, num_children> m_ts{};
    std::size_t m_ctot{};
    std::size_t m_ttot{};
    std::size_t m_depth;
    double m_d;
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
        // if (!m_children[idx]) {
        //     std::cout << "HERE" << std::endl;}
        return *m_children[idx];
    }

    ProbAr weight_probs(ProbAr const &parent_probs) const {
        // See Eq1. Decomposed from:
        // (c_i-d*t_i + d*t*pp_i)/c
        // to:
        // (c_i/c) - d/c*(t_i + t*pp_i)
        // Because std::transform can only do binary ops
        if (!m_ctot) {
            return parent_probs;
        }
        ProbAr tmp;
        for (int i =0; i < tmp.size(); ++i) {
            tmp[i] = (m_cs[i] - m_d*m_ts[i] + m_d*m_ttot*parent_probs[i])/m_ctot;
        }
        // std::transform(parent_probs.begin(), parent_probs.end(), m_ts.begin(), tmp.begin(),
        //                [this](auto const &pp, auto const &tus) {
        //                    return m_d/m_ctot*(tus + m_ttot*pp);
        //                });
        // std::transform(tmp.begin(), tmp.end(), m_cs.begin(), tmp.begin(),
        //                [this](auto const &rhs, auto const &cus) {
        //                    return cus/m_ctot - rhs;
        //                });
        return tmp;

    }
    ProbAr get_prior() const {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), static_cast<double>(1)/num_children);
        return ret;
    }
    ProbAr get_probs_rec(IdxContext const &ctx, ProbAr const &parent_probs) const {
        // Think about how to unroll context to maintain const-ness
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
        // Flip T, see old value. In Chinese Restaurant Parlance: we didn't make a new table
        auto old_t = std::exchange(m_ts[sym], 1);
        m_ttot += !old_t;
        return old_t;
    }
    bool learn_down(IdxContext const &ctx, idx_t sym, ProbAr const &parent_probs) {
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
    using count_t = std::size_t;
    using idx_t = std::size_t;
    using sym_t = typename Alphabet::sym_t;
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


#include "corpus.hpp"
#include "model_utils.hpp"
#include "ac.hpp"
#include "limit_mem.hpp"
template <typename ModelCtorT>
void correctness_and_entropy_test(ModelCtorT ctor) {
    static_assert(std::is_same_v<typename decltype(ctor())::Alphabet::sym_t,bit_t>);
    auto contents = load_file_in_memory(cantbry_name_to_path.at("fields.c"));
    BitVec compressed;
    {
        StreamingACEnc ac(compressed, ctor());
        for (auto const &sym: contents.bits) {
            ac.encode(sym);
        }
    }
    std::cout << "Compression: " << contents.bits.size() << " -> " <<compressed.size() << std::endl;
    StreamingACDec ac(std::move(compressed), ctor());
    for (auto const &gt : contents.bits) {
        assert(ac.decode() == gt);
    }
    auto res = entropy_of_model(contents.bits, ctor());
    std::cout << "Entropy: " << res.H << std::endl;
    std::cout << "Size: " << res.model.footprint().mib() << "MiB" << std::endl;
}
int main() {
    limit_gb(3);
    correctness_and_entropy_test([]() {
        return SequenceMemoizer<BitAlphabet>(100);
    });
    // auto sm = SequenceMemoizer<ByteAlphabet>(5);
    std::cout <<"hi" << std::endl;
}
