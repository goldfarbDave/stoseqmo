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
    auto [entropy, model] = entropy_of_model(contents.bits, ctor());
    std::cout << "Entropy: " << entropy << std::endl;
    std::cout << "Size: " << model.footprint().mib() << "MiB" << std::endl;
}

int main() {
    limit_gb(3);
    auto dmax{9};
    // auto contents = load_file_in_memory(cantbry_name_to_path.at("xargs.1"));
    // auto [H, model] = entropy_of_model(contents.bytes, VolfModel<ByteAlphabet>(9, 15.0));
    // std::cout << H << " " << make_size_string(model) << std::endl;
    correctness_and_entropy_test([]() {
        return VolfModel<BitAlphabet>(10, 15.0);
    });
    correctness_and_entropy_test([]() {
        return HashModel<BitAlphabet>(1000, 10);
    });
    // for (auto const &name: calgary_names) {
    //     auto contents = load_file_in_memory(calgary_name_to_path.at(name));
    //     std::cout << name << " ";
    // }
    // std::cout << "File,Alphabet,Depth,Entropy" << std::endl;
    // for (auto const & name: calgary_names) {
    //     auto const path = calgary_name_to_path.at(name);
    //     auto contents = load_file_in_memory(path);
    //     for (int depth = 1; depth < dmax; ++depth) {
    //         std::cout << name << ",Bit," << depth << ","
    //                   << std::setprecision(7)
    //                   << entropy_of_model(contents.bits, VolfModel<BitAlphabet>(depth, 15.0))
    //                   << std::endl;
    //     }
    //     for (int depth = 1; depth < dmax; ++depth) {
    //         std::cout << name << ",Byte," << depth << ","
    //                   << std::setprecision(7)
    //                   << entropy_of_model(contents.bytes, VolfModel<ByteAlphabet>(depth, 15.0))
    //                   << std::endl;
    //     }
    // }

}
