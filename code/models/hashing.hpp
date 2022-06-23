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
template <template<std::size_t> class HistogramT, typename AlphabetT>
class BottomUpContextHashTable {
public:
    using Alphabet = AlphabetT;
    using Node = HistogramT<AlphabetT::size>;
private:
    std::size_t m_depth{};
    std::vector<Node> m_table;
    using ProbAr = std::array<double, Alphabet::size>;
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
    BottomUpContextHashTable(std::size_t depth, std::size_t num_entries)
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
        return std::accumulate(std::next(hashed_idxs.rbegin()),
                               hashed_idxs.rend(),
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
// Hashing ideas: FNV, pearson
// template <typename AlphabetT>
// class HashModel {
// public:
//     using Alphabet = AlphabetT;
// private:
//     using count_t = std::size_t;
//     using idx_t = std::size_t;
//     using sym_t = typename Alphabet::sym_t;
//     using histogram_t = Histogram<count_t, Alphabet::size>;
//     MemoryDeque<idx_t> m_past_idxs;
//     ContextHashTable<idx_t, histogram_t> m_hashtable;

// public:
//     auto get_probs() const {
//         return m_hashtable.get_probs(m_past_idxs.view());
//     }
//     Footprint footprint() const {
//         return Footprint{.num_nodes=m_hashtable.size(),
//                          .node_size=sizeof(histogram_t)};
//     }
//     HashModel (std::size_t num_entries, std::size_t depth) : m_past_idxs{depth}, m_hashtable{num_entries} {}
//     void learn(sym_t sym) {
//         auto idx = Alphabet::to_idx(sym);
//         auto view = m_past_idxs.view();
//         m_hashtable.learn(view, idx);
//         m_past_idxs.push_back(idx);
//     }
// };
