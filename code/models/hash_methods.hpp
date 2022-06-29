#pragma once
#include <vector>
#include "model_ctx.hpp"

// Boost's impl
template <class T>
void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}


class RandomLookup {
public:
    using hash_t = size_t;
private:
    std::size_t num_entries;
    //std::size_t depth;
    std::size_t start_seed{0};
public:
    RandomLookup(std::size_t /*depth_*/, std::size_t table_size) : num_entries{table_size}
                                                                   //, depth{depth_}
        {}
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<std::size_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{start_seed};
        ret.push_back(seed);
        while (ctx) {
            hash_combine(seed, ctx.pop());
            ret.push_back(seed);
        }
        return ret;
    }
    std::size_t hash_to_idx(hash_t const &hash) const {
        return hash % num_entries;
    }
};
class PureZCTXLookup {
public:
    using hash_t = size_t;
private:
    std::size_t num_entries;
    //std::size_t depth;
    std::size_t start_seed{0};

    template <class T>
    inline void guarded_hash_combine(std::size_t& seed, const T& v, std::size_t guard_seed) const {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        if (seed == guard_seed) {
            seed += 1;
        }
    }
public:
    PureZCTXLookup(std::size_t /*depth_*/, std::size_t table_size) : num_entries{table_size}
                                                                   // , depth{depth_}
        {}
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<hash_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{start_seed};
        ret.push_back(seed);
        while (ctx) {
            guarded_hash_combine(seed, ctx.pop(), start_seed);
            ret.push_back(seed);
        }
        return ret;
    }
    std::size_t hash_to_idx(hash_t const &hash) const {
        if (hash == start_seed) {
            return 0;
        }
        auto loc = hash % num_entries;
        return loc ? loc : 1;
    }
};


// Let's only let equal-sized contexts share the same buckets, and
// then do random allocation from there.
// It seems reasonable to give exponential spacing?
class LengthBucketLookup {
private:
    struct Key {
        int depth;
        std::size_t hash;
    };
public:
    using hash_t = Key;
    using se_t = std::pair<std::size_t, std::size_t>;
    se_t make_bounded(std::size_t start, std::size_t end) const {
        // Using num_entries/4 seems to be the best after a few runs over calgary
        const auto lower = m_num_entries >> 2;
        const auto upper = m_num_entries;
        if (end <= start) return std::make_pair(lower, upper);
        return std::make_pair(std::min(lower, start),
                              std::min(upper, end));
    }
public:
    std::size_t m_num_entries;
    std::size_t m_depth;
    std::size_t m_start_seed{0};

    se_t start_and_end_from_depth(int depth) const {
        constexpr auto p = 8;
        if (depth == 0) return make_bounded(0, 1);
        if (depth == 1) return make_bounded(1, (1UL << p)-1);
        auto start = (1UL << ((depth-1)*p))-1;
        auto end = (1UL << ((depth)*p))-1;
        return make_bounded(start, end);
    }
public:
    LengthBucketLookup(std::size_t depth_, std::size_t table_size) : m_num_entries{table_size}
                                                                   , m_depth{depth_} {}
    std::size_t hash_to_idx(hash_t const& hash) const {
        auto [start, end] = start_and_end_from_depth(hash.depth);
        auto width = end-start;
        auto candidate = start + (hash.hash % width);
        return candidate;
    }
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<hash_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{m_start_seed};
        auto depth{0};
        ret.push_back(Key{.depth=depth++, .hash=seed});
        while (ctx) {
            hash_combine(seed, ctx.pop());
            ret.push_back(Key{.depth=depth++, .hash=seed});
        }
        return ret;
    }
};

class DepthSeededLookup {
public:
    using hash_t = size_t;
private:
    std::size_t m_num_entries;
    std::size_t m_depth;
    std::vector<size_t> m_inits{};
public:
    DepthSeededLookup(std::size_t depth_, std::size_t table_size) : m_num_entries{table_size}, m_depth{depth_} {
        /*
          Initial value from:
          (progn
          (random "seed")
          (random (1- (ash 1 64))))
         */
        auto seed{13678653399469710531UL};
        m_inits.push_back(seed);
        for (auto i =0UL; i < m_depth; i++) {
            hash_combine(seed, i);
            m_inits.push_back(seed);
        }
    }
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<std::size_t> ret;
        auto depth = 0UL;
        ret.reserve(ctx.size()+1);
        auto seed{m_inits[depth++]};
        ret.push_back(seed);
        while (ctx) {
            hash_combine(seed, m_inits[depth++]);
            hash_combine(seed, ctx.pop());
            ret.push_back(seed);
        }
        return ret;
    }
    std::size_t hash_to_idx(hash_t const &hash) const {
        return hash % m_num_entries;
    }
};
