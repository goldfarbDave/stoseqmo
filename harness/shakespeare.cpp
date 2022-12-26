#include <iostream>
#include <sstream>
#include <iomanip>

#include "corpus.hpp"
#include "model_utils.hpp"

#include "hashing.hpp"
#include "volfctw.hpp"

#include "threadpool.hpp"
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
        ss << fn << "," << mn << "," << fs << "," << ms.num_nodes << ","
           << std::setprecision(15)
           << entropy;
        return ss.str();
    }
};
constexpr auto DEPTH = 6;
LineItem do_ctw(std::vector<byte_t> const &bytes, std::string const &name) {
    auto res = entropy_of_model(bytes, VolfModel<ByteAlphabet>(DEPTH, 15.0));
    LineItem ctw_li{.fn=name,
                    .mn="CTW",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H,
    };
    return ctw_li;
}
LineItem do_hash(std::vector<byte_t> const &bytes, std::string const& name, int tab_size) {
    auto res = entropy_of_model(bytes, HashModel<ByteAlphabet>(tab_size, DEPTH));
    std::ostringstream ss;
    LineItem hm_li{.fn = name,
                   .mn="HashCTW",
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H
    };
    return hm_li;
}
LineItem do_amnesia_ctw(std::vector<byte_t> const &bytes, std::string const& name, int tab_size) {
    auto res = entropy_of_model(bytes, AmnesiaVolfModel<ByteAlphabet>(DEPTH, 15.0, tab_size));
    std::ostringstream ss;
    LineItem hm_li{.fn = name,
                   .mn="AmnesiaCTW",
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H
    };
    return hm_li;
}

int main() {
    std::cout << LineItem{}.header() << std::endl;

    std::vector<std::packaged_task<LineItem()>> stvec;
    auto contents = load_shakespeare();
    auto name = "shakespeare";
    auto ctwfunc = [name, contents]() {
        return do_ctw(contents.bytes, name);
    };
    auto hfunc = [name, contents](int i) {
        return do_hash(contents.bytes, name, i);
    };
    auto amnfunc = [name, contents](int i) {
        return do_amnesia_ctw(contents.bytes, name, i);
    };
    // stvec.emplace_back([ctwfunc](){return ctwfunc();});
    // for (int i =7; i < 22; ++i) {
    //     stvec.emplace_back([hfunc, i](){return hfunc(1<<i);});
    // }
    // for (int i = 9; i< 20; ++i ) {
    //     stvec.emplace_back([amnfunc, i](){return amnfunc(1<<i);});
    // }
    // Generated int(nplinspace)
    // std::vector<int> tsizes = {524288, 699050, 873813, 1048576, 1223338, 1398101, 1572864, 1747626, 1922389, 2097152};
    // for (auto const& tsize: tsizes) {
    //     stvec.emplace_back([hfunc, tsize](){return hfunc(tsize);});
    // }

    std::vector<std::future<LineItem>> futvec;
    Threadpool tp(1);
    for (auto &st: stvec) {
        futvec.push_back(tp.add_task<LineItem>(st));
    }
    for (auto &f: futvec) {
        f.wait();
        std::cout << f.get().line() << std::endl;
    }
}
