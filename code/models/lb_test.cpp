#include <iostream>
#include "hash_methods.hpp"
#include "model_ctx.hpp"
int main() {
    // Pure zero order and first order
    PureLowCtxLookup<3> lookup(8, 1<<20UL);
    std::vector<idx_t> ctxvec;
    for (int i = 0; i < 8; ++i) {
        ctxvec.push_back(255);
    }
    auto ctx = Context<idx_t>(ctxvec.begin(), ctxvec.end());
    auto hashvec = lookup.ctx_to_hashes(ctx);
    for (auto const &hash: hashvec) {

        auto idx = lookup.hash_to_idx(hash);
        std::cout << "Depth: " << hash.depth << " hash: " << hash.hash << " pure items: {";
        for (auto const &it: hash.pure_items) {
            std::cout << it << ",";
        }
        std::cout << "}: "<< idx << std::endl;

    }

}
