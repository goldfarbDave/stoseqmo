#pragma once
#include <vector>
#include <limits>
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
class FNVLookup {
    // From https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    // Uses FNV-1A
public:
    using hash_t = size_t;
private:
    std::size_t num_entries;
    //std::size_t depth;
    static constexpr std::size_t fnv_offset{0xcbf29ce484222325};
    static constexpr std::size_t fnv_prime{0x100000001b3};
    static void fnv_hash_combine(std::size_t &acc, const std::size_t& new_val) {
        static_assert(std::numeric_limits<std::size_t>::digits == 64);
        for (int i = 0; i < 64; i+=8) {
            auto byte = (new_val >> i) & 0xFF;
            acc ^= byte;
            acc *= fnv_prime;
        }
    }

    
public:
    FNVLookup(std::size_t /*depth_*/, std::size_t table_size) : num_entries{table_size}
                                                                   //, depth{depth_}
        {}
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<std::size_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{fnv_offset};
        ret.push_back(seed);
        while (ctx) {
            fnv_hash_combine(seed, ctx.pop());
            ret.push_back(seed);
        }
        return ret;
    }
    std::size_t hash_to_idx(hash_t const &hash) const {
        return hash % num_entries;
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
template <std::size_t low_ctx_end>
class PureLowCtxLookup {
public:
//private:
    struct Key {
        std::size_t depth;
        std::size_t hash;
        std::array<std::size_t, 2> pure_items;
    };
public:
    using hash_t = Key;
    using se_t = std::pair<std::size_t, std::size_t>;
public:
    std::size_t m_num_entries;
    std::size_t m_depth;
    std::size_t m_start_seed{0};
    std::size_t m_hash_start{};
    std::size_t m_hash_size{};
public:
    PureLowCtxLookup(std::size_t depth_, std::size_t table_size) : m_num_entries{table_size}
                                                                 , m_depth{depth_}{
        static_assert(low_ctx_end <= 3);
        static_assert(low_ctx_end > 0);
        assert(low_ctx_end < m_depth);
        switch(low_ctx_end) {
        case 1: {
            m_hash_start = 1;
            break;
        }
        case 2: {
            m_hash_start = (256+1);
            break;
        }
        case 3: {
            m_hash_start = (256+1)*256;
            break;
        }
        }
        m_hash_size = m_num_entries-m_hash_start;
        assert(m_hash_start < m_num_entries);
    }
    std::size_t hash_to_idx(hash_t const& hash) const {
        auto const d = hash.depth;
        if (d < low_ctx_end) {
            // appropriate offset is
            auto p_idx = 0UL;
            if (d == 0) return 0;
            p_idx = 1 + hash.pure_items[0];
            if (d == 1) return p_idx;
            return p_idx*256 + hash.pure_items[1];
        }
        return  m_hash_start+ (hash.hash % m_hash_size) ;

    }
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<hash_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{m_start_seed};
        auto depth{0};
        std::array<std::size_t, 2> items{};
        ret.push_back(Key{.depth=static_cast<size_t>(depth++), .hash=seed, .pure_items=items});
        for (std::size_t i = 0; i <low_ctx_end-1; ++i) {
            if (!ctx) break;
            auto const ctx_item = ctx.pop();
            items[i] = ctx_item;
            hash_combine(seed, ctx_item);
            ret.push_back(Key{.depth=static_cast<size_t>(depth++), .hash=seed, .pure_items=items});
        }
        // Use standard method for the rest
        while (ctx) {
            hash_combine(seed, ctx.pop());
            ret.push_back(Key{.depth=static_cast<size_t>(depth++), .hash=seed, .pure_items=items});
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
