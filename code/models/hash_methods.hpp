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
    std::size_t depth;
    std::size_t start_seed{0};
public:
    RandomLookup(std::size_t depth_, std::size_t table_size) : num_entries{table_size}, depth{depth_} {}
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
    std::size_t depth;
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
    PureZCTXLookup(std::size_t depth_, std::size_t table_size) : num_entries{table_size}, depth{depth_} {}
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
private:
    std::size_t num_entries;
    std::size_t depth;
    std::pair<std::size_t, std::size_t> start_and_end_from_depth(std::size_t dep) {

    }
public:
    LengthBucketLookup(std::size_t depth, std::size_t table_size) : num_entries{table_size}, depth{depth} {}
    std::size_t hash_to_idx(hash_t const& hash) const {

    }
    std::vector<hash_t> ctx_to_hashes(IdxContext ctx) const {

    }


}
