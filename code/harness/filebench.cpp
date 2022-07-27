#include <iostream>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "models.hpp"
#include "threadpool.hpp"

#include "file_utils.hpp"
constexpr auto DEPTH = 8;

template<template<typename AlphabetUsed> class ModelT>
auto plain_factory(std::string const &str) {
    return [str](){return std::make_pair(str, ModelT<ByteAlphabet>(DEPTH));};
}
template<template<typename AlphabetUsed> class ModelT>
auto tab_factory(std::string const &str, std::size_t tab_size) {
    return [str, tab_size](){
        return std::make_pair(str,
                              ModelT<ByteAlphabet>(DEPTH, tab_size));};
}
std::map<std::string, std::function<std::vector<Task>()>> name_to_task_map = {
    {"calgary", get_calgary_tasks},
    {"cantbry", get_cantbry_tasks},
    {"shakespeare", get_shakespeare_tasks}
};

int main(int argc, char *argv[]) {
    limit_gb(5);
    // Wrap threadpool
    Threadpool tp(3);

    auto const tasks = [argc, argv] () {
        if (argc == 1) {
            std::cerr << "No filename provided, using calgary\n";
            return get_calgary_tasks();
        } else if (argc == 2) {
            return name_to_task_map.at(argv[1])();
        } else {
            std::cerr << "Expected one file name\n";
            exit(1);
        }
    }();



    std::vector<std::packaged_task<LineItem()>> stvec;
    std::cout << LineItem{}.header() << std::endl;
    for (auto const &task: tasks) {
        auto wrap_task = [task, &stvec](auto ctor){stvec.emplace_back([task, ctor](){return run_task(task, ctor);});};
        // Convinience macros for more ergonomic typing. Could do XMacro, but it's nice to have comment-out-able list
// #define pw(str, mod) wrap_task(plain_factory<mod>(str))
// #define tw(str, mod) wrap_task(tab_factory<mod>(str, log_tab_size))
#define STRINGIFY(x) #x
#define plain(str) wrap_task(plain_factory<str##Model>(#str))
#define hash(str) wrap_task(tab_factory<Hash##str##Model>(STRINGIFY(Hash##str),\
                                                          1UL << log_tab_size))
#define amnesia(str) wrap_task(tab_factory<Amnesia##str##Model>(STRINGIFY(Amnesia##str),\
                                                                1UL <<log_tab_size))

        plain(CTW);
        // plain(SM1PF);
        // plain(SMUKN);
        // plain(PPMDP);
        // plain(PPMDPFull);
        for (int log_tab_size = 7; log_tab_size < 8; ++log_tab_size) {
            // hash(CTW);
            // hash(SM1PF);
            // hash(SMUKN);
            // hash(PPMDP);
            // hash(PPMDPFull);
            // amnesia(CTW);
            // amnesia(SM1PF);
            // amnesia(SMUKN);
            // amnesia(PPMDP);
            // amnesia(PPMDPFull);
        }
#undef amnesia
#undef hash
#undef plain
#undef STRINGIFY
        break;
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
