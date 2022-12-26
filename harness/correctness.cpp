#include <iostream>
#include "limit_mem.hpp"
#include "ac.hpp"
#include "corpus.hpp"
#include "model_utils.hpp"
#include "models.hpp"
#include "utils.hpp"
template <typename ModelCtorT>
void correctness_and_entropy_test(ModelCtorT ctor) {
    // static_assert(std::is_same_v<typename decltype(ctor())::Alphabet::sym_t,byte_t>);
    auto contents = load_file_in_memory(cantbry_name_to_path.at("fields.c"));
    // auto contents = load_file_in_memory("../data/abc1bit/abc1bit.txt");
    // auto contents = load_file_in_memory("../testfile");

    // auto contents = load_file_in_memory(calgary_name_to_path.at("bib"));
    // auto contents = load_file_in_memory(calgary_name_to_path.at("pic"));
    // auto contents = load_file_in_memory(cantbry_name_to_path.at("kennedy.xls"));
    // auto contents = load_shakespeare();
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
        // auto res = lprun(contents.bytesPEOF, ctor());
        auto fp = res.model.footprint();
        std::cout << "Entropy: " << res.H << std::endl;
        std::cout << "Size: " << fp.mib()
                  << "MiB (" << fp.num_nodes << ", " <<(fp.is_constant ? "capped" : "uncapped") << ")"<< std::endl;
    }

}

#define TEST(...) correctness_and_entropy_test([](){return __VA_ARGS__ ;})
#define TESTWC(closure, ...) correctness_and_entropy_test([closure](){return __VA_ARGS__ ;})
int main() {
    limit_gb(3);
    TEST(CTWModel<ByteAlphabet>(8));
    TEST(HashSMUKNModel<ByteAlphabet>(20, 1UL << 8));
    {
        CoinFlipper cf;
        TESTWC(&cf, NewHashSMUKNModel<ByteAlphabet>(20, 1UL << 8, cf));
    }
    TEST(SMUKNModel<ByteAlphabet>(8));
    {
        CoinFlipper cf;
        TESTWC(&cf, NewSMUKNModel<ByteAlphabet>(8, cf));
    }

    // correctness_and_entropy_test([]() {
    //     return Pure0HashCTWModel<ByteAlphabet>(8, 1UL<<20);
    // });
    // correctness_and_entropy_test([]() {
    //     return Pure1HashCTWModel<ByteAlphabet>(8, 1UL<<20);
    // });
    // correctness_and_entropy_test([]() {
    //     return Pure2HashCTWModel<ByteAlphabet>(8, 1UL<<20);
    // });
    // correctness_and_entropy_test([]() {
    //     return Pure0HashSMUKNModel<ByteAlphabet>(8, 1UL<<20);
    // });
    // correctness_and_entropy_test([]() {
    //     return Pure1HashSMUKNModel<ByteAlphabet>(8, 1UL<<20);
    // });
    // correctness_and_entropy_test([]() {
    //     return Pure2HashSMUKNModel<ByteAlphabet>(8, 1UL<<20);
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
    // correctness_and_entropy_test([]() {
    //     return DepthSeededHashCTWModel<ByteAlphabet>(8, 24983UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return SM1PFModel<ByteAlphabet>(8);
    // });
    // correctness_and_entropy_test([]() {
    //     return PPMDPModel<ByteAlphabet>(8);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashPPMDPModel<ByteAlphabet>(8, 30000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return FullPPMDPModel<ByteAlphabet>(8);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashFullPPMDPModel<ByteAlphabet>(8, 10000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashSM1PFModel<ByteAlphabet>(8, 10'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return NBSM1PFModel<ByteAlphabet>(8);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashNBSM1PFModel<ByteAlphabet>(8, 10'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return DepthSeededHashNBSM1PFModel<ByteAlphabet>(8, 10'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return NBSM1PFModel<ByteAlphabet>(2);
    // });
    // correctness_and_entropy_test([]() {
    //     return HashNBSM1PFModel<ByteAlphabet>(8, 30'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return DepthSeededHashNBSM1PFModel<ByteAlphabet>(8, 30'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return  HashPureZCTXSMUKNModel<ByteAlphabet>(15, 30'000UL);
    // });
    // correctness_and_entropy_test([]() {
    //     return  HashPureZCTXSM1PFModel<ByteAlphabet>(15, 30'000UL);
    // });
}
