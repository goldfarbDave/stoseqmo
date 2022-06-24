#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include "model_ctx.hpp"
#include "model_mem.hpp"
#include "hash_utils.hpp"
// Hash Table that calls histogram methods from bottom of context, to top
// Follows semantics of CTW
template <std::size_t N>
class HashVolf {
public:
    constexpr static std::size_t size = N;
    using Node = VolfHistogram<size>;
private:
    std::size_t m_depth{};
    std::vector<Node> m_table;
    using ProbAr = std::array<double, size>;
    Node& lookup(std::size_t hash) {
        return m_table[hash % m_table.size()];
    }
    Node const & lookup(std::size_t hash) const {
        return m_table[hash % m_table.size()];
    }

    std::size_t start_seed{0};
    std::vector<std::size_t> ctx_to_hashed_idxs(IdxContext ctx) const {
        std::vector<std::size_t> idxs;
        idxs.reserve(m_depth);
        auto seed{start_seed};
        idxs.push_back(start_seed);
        while (ctx) {
            hash_combine(seed, ctx.pop());
            idxs.push_back(seed);
        }
        return idxs;
    }
public:
    HashVolf(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_table(num_entries) {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        std::accumulate(std::next(hashed_idxs.rbegin()),
                        hashed_idxs.rend(),
                        lookup(hashed_idxs.back()).learn(sym),
                        [this, sym](ProbAr const &acc, auto const &idx) {
                            return lookup(idx).learn(sym, acc);
                        });
    }
    ProbAr get_probs(IdxContext ctx) const {
        // Let's be naive and build up our counts, then iterate backwards:
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        return std::accumulate(std::next(hashed_idxs.crbegin()),
                               hashed_idxs.crend(),
                               lookup(hashed_idxs.back()).get_probs(),
                               [this](ProbAr const &acc, auto const &idx) {
                                   return lookup(idx).transform_probs(acc);
                               });
    }

    Footprint footprint() const {
        return Footprint{.num_nodes=m_table.size(),
                         .node_size=sizeof(Node),
                         .is_constant=true};
    }
};

template <std::size_t N>
class HashSequenceMemoizer {
public:
    constexpr static std::size_t size = N;
    using Node = SMHistogram<N>;
private:
    std::size_t m_depth{};
    std::vector<Node> m_table;
    using ProbAr = std::array<double, size>;
    Node& lookup(std::size_t hash) {
        return m_table[hash % m_table.size()];
    }
    Node const & lookup(std::size_t hash) const {
        return m_table[hash % m_table.size()];
    }

    std::size_t start_seed{0};
    struct PathProb {
        std::size_t idx;
        ProbAr parent_prob;
    };
    std::vector<PathProb> ctx_to_path_prob(IdxContext ctx) const {
        std::vector<PathProb> ret;
        ret.reserve(m_depth);
        auto seed{start_seed};
        ret.push_back(PathProb{.idx=seed, .parent_prob=Node::get_prior()});
        auto depth = 0;
        for (auto c = ctx; c; c.pop()) {
            hash_combine(seed, ctx.pop());
            ret.push_back(
                PathProb{.idx=seed,
                         .parent_prob=lookup(seed).transform_probs(ret.back().parent_prob,
                                                                   depth++)});
        }
        return ret;
    }
public:
    HashSequenceMemoizer(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_table(num_entries) {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hash_pps = ctx_to_path_prob(ctx);
        for (auto itr = hash_pps.rbegin(); itr != hash_pps.rend(); ++itr) {
            if (lookup(itr->idx).update_counts(sym, itr->parent_prob)) {
                break;
            }
        }
    }
    ProbAr get_probs(IdxContext ctx) const {
        auto seed{start_seed};
        auto depth = 0;
        auto ret = lookup(seed).transform_probs(Node::get_prior(), depth++);
        while (ctx) {
            hash_combine(seed, ctx.pop());
            ret = lookup(seed).transform_probs(ret, depth++);
        }
        return ret;
    }
    Footprint footprint() const {
        return Footprint{.num_nodes=m_table.size(),
                         .node_size=sizeof(Node),
                         .is_constant=true};
    }
};
