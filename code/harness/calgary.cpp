#include <iostream>
#include <sstream>
#include <iomanip>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "models.hpp"
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
// LineItem do_ctw(std::vector<byte_t> const &bytes, std::string const &name) {
//     auto res = entropy_of_model(bytes, VolfModel<ByteAlphabet>(DEPTH, 15.0));
//     LineItem ctw_li{.fn=name,
//                     .mn="CTW",
//                     .fs=bytes.size(),
//                     .ms=res.model.footprint(),
//                     .entropy=res.H,
//     };
//     return ctw_li;
// }
// LineItem do_hashctw(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
//     auto res = entropy_of_model(bytes, HashModel<ByteAlphabet>(1<<log_tab_size, DEPTH));
//     LineItem hm_li{.fn = name,
//                    .mn="HashCTW",
//                    .fs=bytes.size(),
//                    .ms=res.model.footprint(),
//                    .entropy=res.H
//     };
//     return hm_li;
// }
LineItem do_amnesiactw(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes, AmnesiaVolfCTWModel<ByteAlphabet>(DEPTH, 1<<log_tab_size));
    LineItem am_li{.fn = name,
                   .mn="AmnesiaCTW",
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H
    };
    return am_li;
}
LineItem do_smukn(std::vector<byte_t> const &bytes, std::string const &name) {
    auto res = entropy_of_model(bytes, SMUKNModel<ByteAlphabet>(DEPTH));
    LineItem sm_li{.fn=name,
                   .mn="SMUKN",
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H,
    };
    return sm_li;
}

LineItem do_hashsmukn(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes,
                                HashSMUKNModel<ByteAlphabet>(DEPTH,
                                                             1UL<<log_tab_size));
    LineItem hsm_li{.fn = name,
                    .mn="HashSMUKN",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H
    };
    return hsm_li;
}
LineItem do_sm1pf(std::vector<byte_t> const &bytes, std::string const &name) {
    auto res = entropy_of_model(bytes, SM1PFModel<ByteAlphabet>(DEPTH));
    LineItem sm_li{.fn=name,
                   .mn="SM1PF",
                   .fs=bytes.size(),
                   .ms=res.model.footprint(),
                   .entropy=res.H,
    };
    return sm_li;
}

LineItem do_hashsm1pf(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes,
                                HashSM1PFModel<ByteAlphabet>(DEPTH,
                                                             1UL<<log_tab_size));
    LineItem hsm_li{.fn = name,
                    .mn="HashSM1PF",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H
    };
    return hsm_li;
}
LineItem do_amnesiasm1pf(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes,
                                AmnesiaSM1PFModel<ByteAlphabet>(DEPTH,
                                                                1UL<<log_tab_size));
    LineItem hsm_li{.fn = name,
                    .mn="AmnesiaSM1PF",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H
    };
    return hsm_li;
}
LineItem do_amnesiasmukn(std::vector<byte_t> const &bytes, std::string const& name, int log_tab_size) {
    auto res = entropy_of_model(bytes,
                                AmnesiaSMUKNModel<ByteAlphabet>(DEPTH,
                                                             1UL<<log_tab_size));
    LineItem hsm_li{.fn = name,
                    .mn="AmnesiaSMUKN",
                    .fs=bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H
    };
    return hsm_li;
}
int main() {
    limit_gb(5);
    std::cout << LineItem{}.header() << std::endl;
    Threadpool tp(4);

    std::vector<std::packaged_task<LineItem()>> stvec;
    for (auto const & name: calgary_names) {
        // auto ctwfunc = [name]() {
        //     auto const path = calgary_name_to_path.at(name);
        //     auto const contents = load_file_in_memory(path);
        //     return do_ctw(contents.bytes, name);
        // };
        // auto hctwfunc = [name](int i) {
        //     auto const path = calgary_name_to_path.at(name);
        //     auto const contents = load_file_in_memory(path);
        //     return do_hashctw(contents.bytes, name, i);
        // };
        auto actwfunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_amnesiactw(contents.bytes, name, i);
        };
        // stvec.emplace_back([ctwfunc](){return ctwfunc();});
        for (int i =7; i < 19; ++i) {
            // stvec.emplace_back([hctwfunc, i](){return hctwfunc(i);});
            stvec.emplace_back([actwfunc, i](){return actwfunc(i);});
        }

        auto smuknfunc = [name]() {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_smukn(contents.bytes, name);
        };
        auto sm1pffunc = [name]() {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_sm1pf(contents.bytes, name);
        };
        auto hsmuknfunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_hashsmukn(contents.bytes, name, i);
        };
        auto hsm1pffunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_hashsm1pf(contents.bytes, name, i);
        };
        auto asmuknfunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_amnesiasmukn(contents.bytes, name, i);
        };
        auto asm1pffunc = [name](int i) {
            auto const path = calgary_name_to_path.at(name);
            auto const contents = load_file_in_memory(path);
            return do_amnesiasm1pf(contents.bytes, name, i);
        };
        // stvec.emplace_back([smuknfunc](){return smuknfunc();});
        // for (int i=7; i < 19; ++i) {
        //     stvec.emplace_back([asmuknfunc, i](){return asmuknfunc(i);});
        //     stvec.emplace_back([asm1pffunc, i](){return asm1pffunc(i);});
        // }
        // stvec.emplace_back([sm1pffunc](){return sm1pffunc();});
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
