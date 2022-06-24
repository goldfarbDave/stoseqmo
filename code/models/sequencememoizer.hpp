#pragma once
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
#include <unordered_map>
#include <vector>
#include "model_counter.hpp"
#include "model_ctx.hpp"
#include "model_mem.hpp"
#include "model_sequence.hpp"
double get_init_discount(std::size_t depth) {
    // Values from footnote 2
    static constexpr std::array<double, 11> g_discount_ar{0.05, 0.7, 0.8, 0.82, 0.84, 0.88, 0.91, 0.92, 0.93, 0.94, 0.95};
    // Clamp at end of discount ar
    auto idx = std::min(depth, g_discount_ar.size() - 1);
    return g_discount_ar[idx];
}
template <std::size_t num_children>
class SMHistogram {
public:
    static constexpr std::size_t size = num_children;
    using count_t = std::uint8_t;
    using ProbAr = std::array<double, num_children>;
private:
    RescalingCounter<count_t, num_children> m_ccounter;
    std::array<count_t, num_children> m_ts{};
    std::size_t m_ttot{};
    // Tracked externally along with depth
public:
    ProbAr transform_probs(ProbAr const &parent_probs, std::size_t depth) const {
        // See Eq1.
        if (!m_ccounter.total()) {
            return parent_probs;
        }
        auto const discount = get_init_discount(depth);
        ProbAr tmp;
        for (auto i =0UL; i < tmp.size(); ++i) {
            tmp[i] = (m_ccounter[i]
                      - discount*m_ts[i]
                      + (discount
                         * static_cast<double>(m_ttot)
                         * parent_probs[i]))
                / static_cast<double>(m_ccounter.total());
        }
        return tmp;
    }
    static constexpr ProbAr get_prior() {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), 1.0/num_children);
        return ret;
    }

    bool update_counts(idx_t sym, ProbAr const& /*parent_probs*/) {
        // Unconditionally increment C
        m_ccounter.increment(sym);
        // Flip T, see old value. In Chinese Restaurant Parlance: we didn't make a new table
        auto old_t = std::exchange(m_ts[sym], 1);
        m_ttot += !old_t;
        return old_t;
    }
};



// Amortizes node allocation through std::vector resize semantics
template <std::size_t N>
class AmortizedSM {
    // Dynamic storage of children from a central allocation
    static constexpr std::size_t num_children = N;
public:
    using Node=SMHistogram<num_children>;
private:
    using Ptr_t=uint32_t;
    std::vector<Node> m_vec;
    std::unordered_map<Ptr_t, std::array<Ptr_t, num_children>> m_adj;
    std::size_t m_depth;
    inline static constexpr std::array<Ptr_t, num_children> mzeroinit{};
    using ProbAr = std::array<double, num_children>;
    struct IdxAndProb {
        Ptr_t idx;
        ProbAr prob;
    };
    auto get_prob_path(IdxContext const &ctx) {
        std::vector<IdxAndProb> cpps;
        cpps.reserve(m_depth);
        cpps.push_back(IdxAndProb{.idx=0, .prob=Node::get_prior()});
        auto depth = 0;
        for (IdxContext c = ctx; c; c.pop()) {
            // Note we don't form references to m_vec[idx] because we're resizing m_vec.
            auto const parent_idx = cpps.back().idx;
            auto child_idx = m_adj[parent_idx][c.back()];
            // Make child if needed:
            if (!child_idx) {
                assert(m_vec.size() < std::numeric_limits<Ptr_t>::max());
                auto new_idx = static_cast<Ptr_t>(m_vec.size());
                m_adj[parent_idx][c.back()] = new_idx;
                m_adj[new_idx];
                child_idx = new_idx;
                m_vec.emplace_back();
            }
            // Push
            cpps.push_back(IdxAndProb{.idx=child_idx,
                                      .prob=m_vec[child_idx].transform_probs(cpps.back().prob,
                                                                             depth++)});
        }
        return cpps;
    }
public:
    AmortizedSM(std::size_t depth) :m_depth{depth} {
        m_vec.reserve(1<<15);
        m_vec.emplace_back();
        m_adj.reserve(1<<15);
        m_adj[0];
    }
    ProbAr get_probs(IdxContext const &ctx) const {
        auto idx = 0;
        auto depth = 0;
        auto ret= m_vec[0].transform_probs(Node::get_prior(), depth++);
        for (IdxContext c = ctx; c; c.pop()) {
            auto const child_idx = m_adj.at(idx)[c.back()];
            // Leave if can't make child
            if (!child_idx) {
                break;
            }
            ret = m_vec[child_idx].transform_probs(ret, depth++);
            idx = child_idx;
        }
        return ret;
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        auto cpps = get_prob_path(ctx);
        // Work backwards, updating along:
        for (auto itr = cpps.crbegin(); itr != cpps.crend(); ++itr) {
            if (m_vec[itr->idx].update_counts(sym, itr->prob)) {
                break;
            }
        }
    }
    Footprint footprint() const {
        return {.num_nodes = m_vec.size(),
                .node_size = sizeof(Node) + sizeof(mzeroinit),
                .is_constant=false};
    }
};
