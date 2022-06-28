#pragma once
#include <vector>
#include "model_ctx.hpp"
class RandomLookup {
    std::size_t num_entries;
    std::size_t start_seed{0};
private:
    // Boost's impl
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
public:
    RandomLookup(std::size_t table_size) : num_entries{table_size} {}
    std::vector<std::size_t> ctx_to_hashes(IdxContext ctx) const {
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
    std::size_t hash_to_idx(std::size_t hash) const {
        return hash % num_entries;
    }
};
class PureZCTXLookup {
    std::size_t num_entries;
    std::size_t start_seed{0};
private:
    template <class T>
    inline void guarded_hash_combine(std::size_t& seed, const T& v, std::size_t guard_seed) const {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        if (seed == guard_seed) {
            seed += 1;
        }
    }
public:
    PureZCTXLookup(std::size_t table_size) : num_entries{table_size} {}
    std::vector<std::size_t> ctx_to_hashes(IdxContext ctx) const {
        std::vector<std::size_t> ret;
        ret.reserve(ctx.size()+1);
        auto seed{start_seed};
        ret.push_back(seed);
        while (ctx) {
            guarded_hash_combine(seed, ctx.pop(), start_seed);
            ret.push_back(seed);
        }
        return ret;
    }
    std::size_t hash_to_idx(std::size_t hash) const {
        if (hash == start_seed) {
            return 0;
        }
        auto loc = hash % num_entries;
        return loc ? loc : 1;
    }
};
