#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include "model_ctx.hpp"
#include "model_mem.hpp"
// Boost's impl
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
// template <typename El, typename InitFunc, typename BodyFunc>
// using NestedApply_t = decltype(declval<BodyFunc>{}(declval<InitFunc>{}(declval<El>{})));

// template <typename El, typename InitFunc, typename BodyFunc>
// NestedApply_t<C::value_type, InitFunc, BodyFunc>
// pop_accumulate(C cont, InitFunc initfunc, BodyFunc bf) {
//     auto tmp = init_func(cont.back());
//     cont.pop_back();
//     while (!cont.empty()) {
//         body_func
//     }
// }
template <typename K, typename V>
class ContextHashTable {
    // For now, let's assume we can peel size out of our value type
    static_assert(V::size);
    std::vector<V> m_table;
    using ProbAr = std::array<double, V::size>;
    V& lookup(std::size_t hash) {
        return m_table[hash % m_table.size()];
    }
    V const & lookup(std::size_t hash) const {
        return m_table[hash % m_table.size()];
    }

    std::size_t start_seed{0};
    std::vector<std::size_t> ctx_to_hashed_idxs(Context<K> ctx) const {
        std::vector<std::size_t> idxs;
        idxs.reserve(ctx.size());
        auto seed{start_seed};
        idxs.push_back(start_seed);
        while (ctx) {
            hash_combine(seed, ctx.pop());
            idxs.push_back(seed);
        }
        return idxs;
    }
    ProbAr get_prior() const {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), 1.0/V::size);
        return ret;
    }
    std::vector<ProbAr> get_parent_prob_chain(std::vector<std::size_t> const &hashed_idxs) const {
        std::vector<ProbAr> ret;
        ret.reserve(hashed_idxs.size()+1);
        ret.push_back(get_prior());
        auto depth = 0;
        for (auto idx_itr = hashed_idxs.crbegin();
             idx_itr != hashed_idxs.crend();
             idx_itr++) {
            ret.push_back(lookup(*idx_itr).transform_probs(ret.back(), depth++));
        }
        return ret;
    }
public:
    ContextHashTable(std::size_t num_entries) : m_table(num_entries) {}

    void learn(Context<K> const &ctx, K const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        auto parent_prob_chain = get_parent_prob_chain(hashed_idxs);
        // Note, we do a forward traversal through hashed_idxs, and a
        // backwards traversal through parent_prob_chain
        // TODO: IS THIS OFF BY ONE?
        auto back_prob_idx = parent_prob_chain.size()-1;
        for (auto const &idx: hashed_idxs) {
            if (lookup(idx).update_counts(sym, parent_prob_chain[--back_prob_idx])) {
                break;
            }
        }
    }
    ProbAr get_probs(Context<K> ctx) const {
        // Let's be naive and build up our counts, then iterate backwards:
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        auto ret = get_prior();
        auto depth = 0;
        for (auto idx_itr = hashed_idxs.crbegin(); idx_itr != hashed_idxs.crend(); idx_itr++) {
            ret = lookup(*idx_itr).transform_probs(ret, depth++);
        }
        return ret;
    }
    std::size_t size() const {
        return m_table.size();
    }
};
double get_init_discount(std::size_t depth) {
    // Values from footnote 2
    static constexpr std::array<double, 11> g_discount_ar{0.05, 0.7, 0.8, 0.82, 0.84, 0.88, 0.91, 0.92, 0.93, 0.94, 0.95};
    // Clamp at end of discount ar
    auto idx = std::min(depth, g_discount_ar.size() - 1);
    return g_discount_ar[idx];
}
template <typename CountT, std::size_t N>
class SMHistogram {
private:
    using count_t = std::uint8_t;
    using idx_t = std::size_t;
    using IdxContext = Context<idx_t>;
    using ProbAr = std::array<double, N>;
    using Ptr_t = std::uint32_t;
    std::array<count_t, N> m_cs{};
    std::array<count_t, N> m_ts{};
    std::size_t m_ctot{};
    std::size_t m_ttot{};
    std::array<Ptr_t, N> children{};
public:
    ProbAr transform_probs(ProbAr const &parent_probs, std::size_t depth) const {
        if (!m_ctot) {
            return parent_probs;
        }
        auto const discount = get_init_discount(depth);
        ProbAr tmp;
        for (int i =0; i < tmp.size(); ++i) {
            tmp[i] = (m_cs[i] - discount*m_ts[i] + discount*m_ttot*parent_probs[i])/m_ctot;
        }
        return tmp;
    }
    bool update_counts(idx_t sym, ProbAr const& parent_probs) {
        // Unconditionally increment C
        m_cs[sym] += 1;
        m_ctot += 1;
        auto const RESCALE_THRESHOLD = std::numeric_limits<uint8_t>::max();
        if (m_cs[sym] == RESCALE_THRESHOLD) {
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

    static constexpr std::size_t size = N;
};
// Hashing ideas: FNV, pearson
template <typename AlphabetT>
class HashSMModel {
public:
    using Alphabet = AlphabetT;
private:
    using count_t = std::size_t;
    using idx_t = std::size_t;
    using sym_t = typename Alphabet::sym_t;
    using histogram_t = SMHistogram<count_t, Alphabet::size>;
    MemoryDeque<idx_t> m_past_idxs;
    ContextHashTable<idx_t, histogram_t> m_hashtable;

public:
    auto get_probs() const {
        return m_hashtable.get_probs(m_past_idxs.view());
    }
    Footprint footprint() const {
        return Footprint{.num_nodes=m_hashtable.size(),
                         .node_size=sizeof(histogram_t)};
    }
    HashSMModel (std::size_t num_entries, std::size_t depth) : m_past_idxs{depth}, m_hashtable{num_entries} {}
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_hashtable.learn(view, idx);
        m_past_idxs.push_back(idx);
    }
};
