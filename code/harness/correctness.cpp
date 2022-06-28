#include <iostream>
#include "limit_mem.hpp"
#include "ac.hpp"
#include "corpus.hpp"
#include "model_utils.hpp"
#include "models.hpp"
#include "utils.hpp"

template <typename ModelCtorT>
void correctness_and_entropy_test(ModelCtorT ctor) {
    static_assert(std::is_same_v<typename decltype(ctor())::Alphabet::sym_t,byte_t>);
    // auto contents = load_file_in_memory(cantbry_name_to_path.at("fields.c"));
    auto contents = load_file_in_memory(calgary_name_to_path.at("bib"));
    BitVec compressed;
    {
        TimeSection ts{"Enc"};
        StreamingACEnc ac(compressed, ctor());
        for (auto const &sym: contents.bytes) {
            ac.encode(sym);
        }
    }
    auto compressed_size = static_cast<double>(compressed.size());
    std::cout << "Compression: " << contents.bits.size() << " -> " << compressed_size << std::endl;
    std::cout << "bits/Bytes: " << compressed_size/static_cast<double>(contents.bytes.size()) << std::endl;
    {
        StreamingACDec ac(std::move(compressed), ctor());
        for (auto const &gt : contents.bytes) {
            assert(ac.decode() == gt);
        }
    }
    {
        auto res = entropy_of_model(contents.bytes, ctor());
        auto fp = res.model.footprint();
        std::cout << "Entropy: " << res.H << std::endl;
        std::cout << "Size: " << fp.mib()
                  << "MiB (" << fp.num_nodes << ", " <<(fp.is_constant ? "capped" : "uncapped") << ")"<< std::endl;
    }

}

int main() {
    // limit_gb(3);
    // correctness_and_entropy_test([]() {
    //     return HashSMModel<ByteAlphabet>(1<<19, 10);
    // });
    // correctness_and_entropy_test([]() {
    //     return VolfCTWModel<ByteAlphabet>(8);
    // });
    // correctness_and_entropy_test([]() {
    //     return AmnesiaVolfCTWModel<ByteAlphabet>(8, 20'000);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashCTWModel<ByteAlphabet>(8, 24983UL);
    // });
    correctness_and_entropy_test([]() {
        return SMUKNModel<ByteAlphabet>(15);
    });
    correctness_and_entropy_test([]() {
        return HashSMUKNModel<ByteAlphabet>(15, 30'000UL);
    });
    correctness_and_entropy_test([]() {
        return SM1PFModel<ByteAlphabet>(15);
    });
    correctness_and_entropy_test([]() {
        return HashSM1PFModel<ByteAlphabet>(15, 30'000UL);
    });

}
