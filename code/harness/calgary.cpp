#include <iostream>
#include <sstream>
#include <iomanip>
#include "limit_mem.hpp"

#include "ac.hpp"
#include "corpus.hpp"
#include "model_utils.hpp"

#include "hashing.hpp"
#include "volfctw.hpp"

struct LineItem {
    std::string fn;
    std::string mn;
    std::size_t fs;
    Footprint ms;
    double entropy;
    static std::string header() {
        return "File,Meth,FSize,MSize,Entropy";
    }
    std::string line() const {
        std::ostringstream ss;
        ss << fn << "," << mn << "," << fs << "," << ms.num_nodes << "," << std::setprecision(7) << entropy;
        return ss.str();
    }
};

int main() {
    limit_gb(3);
    auto depth{8};
    std::cout << LineItem{}.header() << std::endl;
    for (auto const & name: calgary_names) {
        auto const path = calgary_name_to_path.at(name);
        auto const contents = load_file_in_memory(path);
        auto res = entropy_of_model(contents.bytes, VolfModel<ByteAlphabet>(depth, 15.0));
        LineItem ctw_li{
            .fn=name,
                .mn="CTW",
                .fs=contents.bytes.size(),
                .ms=res.model.footprint(),
                .entropy=res.H,
        };
        std::cout << ctw_li.line() << std::endl;
        for (int i = 7; i < 21; ++i) {
            auto res = entropy_of_model(contents.bytes, HashModel<ByteAlphabet>(1<<i, depth));
            std::ostringstream ss;
            ss << "Hash" << (1<<i);
            LineItem hm_li{
                .fn = name,
                .mn=ss.str(),
                .fs=contents.bytes.size(),
                .ms=res.model.footprint(),
                .entropy=res.H
            };
            std::cout << hm_li.line() << std::endl;
        }
    }
}
