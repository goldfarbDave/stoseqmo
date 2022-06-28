#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include "model_ctx.hpp"
#include "model_mem.hpp"
// Hash Table that calls histogram methods from bottom of context, to top
// Follows semantics of CTW
template <typename HashLookupT, typename HistogramT>
class HasherBottomUp {
public:
    constexpr static std::size_t size = HistogramT::size;
    using Node = HistogramT;
private:
    std::size_t m_depth{};
    HashLookupT m_hasher;
    std::vector<Node> m_table;
    using ProbAr = std::array<double, size>;
    Node& lookup(std::size_t hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const & lookup(std::size_t hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }

    std::size_t start_seed{0};
public:
    HasherBottomUp(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_hasher{num_entries}
        , m_table(num_entries) {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
        std::accumulate(std::next(hashed_idxs.rbegin()),
                        hashed_idxs.rend(),
                        lookup(hashed_idxs.back()).learn(sym),
                        [this, sym](ProbAr const &acc, auto const &idx) {
                            return lookup(idx).learn(sym, acc);
                        });
    }
    ProbAr get_probs(IdxContext ctx) const {
        // Let's be naive and build up our counts, then iterate backwards:
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
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


// Hash table that cllas histogram methods top down, then bottum up
// Follows semantics of SequenceMemoizer
template <typename HashLookupT, typename HistogramT>
class HasherTopDown {
public:
    constexpr static std::size_t size = HistogramT::size;
    using Node = HistogramT;
private:
    std::size_t m_depth{};
    HashLookupT m_hasher;
    std::unique_ptr<CoinFlipper> m_flip{std::make_unique<CoinFlipper>()};
    std::vector<Node> m_table;
    using ProbAr = std::array<double, size>;
    Node& lookup(std::size_t hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const & lookup(std::size_t hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    std::size_t start_seed{0};
    struct PathProb {
        std::size_t hash;
        ProbAr parent_prob;
    };
    std::vector<PathProb> ctx_to_path_prob(IdxContext ctx) const {
        std::vector<PathProb> ret;
        auto hashes = m_hasher.ctx_to_hashes(ctx);
        ret.reserve(m_depth+1);
        ret.push_back(PathProb{.hash=hashes[0], .parent_prob=Node::get_prior()});
        auto depth = 0;
        for (auto hash_itr = std::next(hashes.cbegin());
                                       hash_itr != hashes.cend();
                                       ++hash_itr) {
            ret.push_back(
                PathProb{.hash=*hash_itr,
                         .parent_prob=lookup(*hash_itr).transform_probs(ret.back().parent_prob, depth++)
                });
        }
        return ret;
    }
public:
    HasherTopDown(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_hasher(num_entries)
        , m_table(num_entries, *m_flip) {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hash_pps = ctx_to_path_prob(ctx);
        auto depth{hash_pps.size()};
        for (auto itr = hash_pps.rbegin(); itr != hash_pps.rend(); ++itr) {
            if (lookup(itr->hash).update_counts(sym, itr->parent_prob, depth--)) {
                break;
            }
        }
    }
    ProbAr get_probs(IdxContext ctx) const {
        auto hashes = m_hasher.ctx_to_hashes(ctx);
        auto depth = 0;
        auto const prior = Node::get_prior();
        auto ret = std::accumulate(std::next(hashes.cbegin()), hashes.cend(),
                                   lookup(hashes[0]).transform_probs(prior, depth++),
                                   [&depth, this](auto const &prob_ar, auto const &hash) {
                                       return lookup(hash).transform_probs(prob_ar, depth++);
                                   });
        // Mix:
        std::transform(ret.begin(), ret.end(), prior.begin(), ret.begin(),
                       [](auto const &r, auto const &p) {
                           return (99*r + p)/100;
                       });
        return ret;
    }
    Footprint footprint() const {
        return Footprint{.num_nodes=m_table.size(),
                         .node_size=sizeof(Node),
                         .is_constant=true};
    }
};
