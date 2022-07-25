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
    using hash_t = typename HashLookupT::hash_t;
    std::size_t m_depth{};
    HashLookupT m_hasher;
    std::vector<Node> m_table;
    using ProbAr = decltype(Node::get_prior());
    Node& lookup(hash_t const& hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const & lookup(hash_t const& hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }

    std::size_t start_seed{0};
public:
    HasherBottomUp(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_hasher{depth, num_entries}
        , m_table(num_entries) {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
        auto depth = hashed_idxs.size();
        std::accumulate(std::next(hashed_idxs.rbegin()),
                        hashed_idxs.rend(),
                        lookup(hashed_idxs.back()).learn(sym, depth--),
                        [this, sym, &depth](ProbAr const &acc, auto const &idx) {
                            return lookup(idx).learn(sym, acc, depth--);
                        });
    }
    ProbAr get_probs(IdxContext ctx) const {
        // Let's be naive and build up our counts, then iterate backwards:
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
        auto depth = hashed_idxs.size()-1;
        return std::accumulate(std::next(hashed_idxs.crbegin()),
                               hashed_idxs.crend(),
                               lookup(hashed_idxs.back()).get_probs(),
                               [this, &depth](ProbAr const &acc, auto const &idx) {
                                   return lookup(idx).transform_probs(acc, depth--);
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
    using hash_t = typename HashLookupT::hash_t;
    std::size_t m_depth{};
    HashLookupT m_hasher;
    std::unique_ptr<CoinFlipper> m_flip{std::make_unique<CoinFlipper>()};
    std::vector<Node> m_table;
    using ProbAr = decltype(Node::get_prior());
    Node& lookup(hash_t const &hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const & lookup(hash_t const &hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    struct PathProb {
        hash_t hash;
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
        , m_hasher(depth, num_entries)
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
template <typename HashLookupT, typename HistogramT, PPMUpdatePolicy update_policy>
class HasherProbDownLearnUp {
public:
    constexpr static std::size_t size = HistogramT::size;
    using Node = HistogramT;
private:
    std::size_t m_depth;
    HashLookupT m_hasher;
    std::vector<Node> m_table{};
    using hash_t = typename HashLookupT::hash_t;
    using ProbAr = decltype(Node::get_prior());
    Node& lookup(hash_t const &hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const & lookup(hash_t const &hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
public:
    HasherProbDownLearnUp(std::size_t depth, std::size_t num_entries)
        : m_depth{depth}
        , m_hasher(depth, num_entries)
        , m_table(num_entries) {}

    ProbAr get_probs(IdxContext const &ctx) const {
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
        auto depth = 0;
        auto ret = std::accumulate(hashed_idxs.cbegin(),
                                   hashed_idxs.cend(),
                                   Node::get_prior(),
                                   [&depth, this](ProbAr const &acc, auto const &idx) {
                                       return lookup(idx).transform_probs(acc, depth++);
                                   });
        return ret;
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        auto depth = ctx.size();
        auto hashed_idxs = m_hasher.ctx_to_hashes(ctx);
        auto probs = Node::get_prior();
        // Work backwards, updating along:
        for (auto itr = hashed_idxs.crbegin(); itr != hashed_idxs.crend(); ++itr, --depth) {
            auto res = lookup(*itr).learn(sym, probs, depth);
            probs = res.probs;
            if constexpr (update_policy == PPMUpdatePolicy::ShallowUpdates) {
                if (!res.first_time_seen) {
                    break;
                }
            } else if constexpr (update_policy == PPMUpdatePolicy::FullUpdates) {
                continue;
            } else {
                // Uncovered update policy (ie, might want to add stochastic updates akin to 1PF)
                assert(false);
            }
        }
    }

    Footprint footprint() const {
        return {
            .num_nodes=m_table.size(),
            .node_size=sizeof(Node),
            .is_constant=true
        };
    }
};
