#include <iostream>
#include "limit_mem.hpp"

#include "ac.hpp"
#include "corpus.hpp"
#include "model_utils.hpp"

#include "hashing.hpp"
#include "volfctw.hpp"


template <typename ModelCtorT>
void correctness_and_entropy_test(ModelCtorT ctor) {
    static_assert(std::is_same_v<typename decltype(ctor())::Alphabet::sym_t,bit_t>);
    auto contents = load_file_in_memory(cantbry_name_to_path.at("fields.c"));
    BitVec compressed;
    {
        StreamingACEnc ac(compressed, ctor());
        for (auto const &sym: contents.bits) {
            ac.encode(sym);
        }
    }
    std::cout << "Compression: " << contents.bits.size() << " -> " <<compressed.size() << std::endl;
    StreamingACDec ac(std::move(compressed), ctor());
    for (auto const &gt : contents.bits) {
        assert(ac.decode() == gt);
    }
    auto res = entropy_of_model(contents.bits, ctor());
    std::cout << "Entropy: " << res.H << std::endl;
    std::cout << "Size: " << res.model.footprint().mib() << "MiB" << std::endl;
}

int main() {
    limit_gb(3);
    correctness_and_entropy_test([]() {
        return VolfModel<BitAlphabet>(100, 15.0);
    });
    // correctness_and_entropy_test([]() {
    //     return HashModel<BitAlphabet>(1000, 10);
    // });
}
