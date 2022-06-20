#include <iostream>
#include <sstream>
#include <iomanip>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "hashing.hpp"
#include "volfctw.hpp"
#include "sequencememoizer.hpp"

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
           << std::setprecision(7)
           << entropy;
        return ss.str();
    }
};
constexpr auto DEPTH = 8;
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
    ss << "HashCTW" << (1<<log_tab_size);
    LineItem hm_li{.fn = name,
                   .mn=ss.str(),
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H
    };
    return hm_li;
}
LineItem do_sm(std::vector<byte_t> const &bytes, std::string const &name) {
    auto res = entropy_of_model(bytes, SequenceMemoizerAmort<ByteAlphabet>(DEPTH));
    LineItem sm_li{.fn=name,
                    .mn="SM",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H,
    };
    return sm_li;
}
int main() {
    limit_gb(5);
    std::cout << LineItem{}.header() << std::endl;
    Threadpool tp(1);

    std::vector<std::packaged_task<LineItem()>> stvec;
    for (auto const & name: calgary_names) {
        auto ctwfunc = [name]() {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_ctw(contents.bytes, name);
        };
        auto hfunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_hash(contents.bytes, name, i);
        };
        auto smfunc = [name]() {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_sm(contents.bytes, name);
        };
        stvec.emplace_back([smfunc](){return smfunc();});
        // stvec.emplace_back([ctwfunc](){return ctwfunc();});
        // for (int i =7; i < 19; ++i) {
        //     stvec.emplace_back([hfunc, i](){return hfunc(i);});
        // }
    }

    std::vector<std::future<LineItem>> futvec;
    for (auto &st: stvec) {
        futvec.push_back(tp.add_task<LineItem>(st));
    }
    for (auto &f: futvec) {
        f.wait();
        std::cout << f.get().line() << std::endl;
    }
}
