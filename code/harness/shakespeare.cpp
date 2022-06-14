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
LineItem do_hash(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes, HashModel<ByteAlphabet>(1<<log_tab_size, DEPTH));
    std::ostringstream ss;
    ss << "Hash" << (1<<log_tab_size);
    LineItem hm_li{.fn = name,
                   .mn=ss.str(),
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
    stvec.emplace_back([ctwfunc](){return ctwfunc();});
    for (int i =7; i < 19; ++i) {
        stvec.emplace_back([hfunc, i](){return hfunc(i);});
    }


    std::vector<std::future<LineItem>> futvec;
    Threadpool tp(4);
    for (auto &st: stvec) {
        futvec.push_back(tp.add_task<LineItem>(st));
    }
    for (auto &f: futvec) {
        f.wait();
        std::cout << f.get().line() << std::endl;
    }
}
