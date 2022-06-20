#include <iostream>
#include "limit_mem.hpp"

#include "ac.hpp"
#include "corpus.hpp"
#include "model_utils.hpp"

// #include "hashing.hpp"
#include "hashsm.hpp"
// #include "sequencememoizer.hpp"
#include "volfctw.hpp"


template <typename ModelCtorT>
void correctness_and_entropy_test(ModelCtorT ctor) {
    static_assert(std::is_same_v<typename decltype(ctor())::Alphabet::sym_t,byte_t>);
    auto contents = load_file_in_memory(cantbry_name_to_path.at("fields.c"));
    BitVec compressed;
    {
        StreamingACEnc ac(compressed, ctor());
        for (auto const &sym: contents.bytes) {
            ac.encode(sym);
        }
    }
    std::cout << "Compression: " << contents.bits.size() << " -> " <<compressed.size() << std::endl;
    std::cout << "bits/Bytes: " << static_cast<double>(compressed.size())/contents.bytes.size() << std::endl;

    StreamingACDec ac(std::move(compressed), ctor());
    for (auto const &gt : contents.bytes) {
        assert(ac.decode() == gt);
    }
    auto res = entropy_of_model(contents.bytes, ctor());
    // std::cout << "Entropy: " << res.H << std::endl;
    std::cout << "Size: " << res.model.footprint().mib() << "MiB (" << res.model.footprint().num_nodes << ")"<< std::endl;
}

int main() {
    limit_gb(3);
    correctness_and_entropy_test([]() {
        return HashSMModel<ByteAlphabet>(1<<19, 10);
    });

    // correctness_and_entropy_test([]() {
    //     return VolfModel<ByteAlphabet>(20, 15.0);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashModel<ByteAlphabet>(1000, 10);
    // });
    // correctness_and_entropy_test([]() {
    //     return SequenceMemoizerAmort<ByteAlphabet>(10);
    // });
}
