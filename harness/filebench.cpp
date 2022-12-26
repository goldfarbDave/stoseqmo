#include <iostream>
#include <string>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "models.hpp"
#include "threadpool.hpp"

#include "file_utils.hpp"

std::map<std::string, std::function<std::vector<Task>()>> name_to_task_map = {
    {"calgary", get_calgary_tasks},
    {"cantbry", get_cantbry_tasks},
    {"shakespeare", get_shakespeare_tasks},
};

int main(int argc, char *argv[]) {
    // limit_gb(5);
    int const num_threads = 8;//std::thread::hardware_concurrency();
    std::cerr << "Executing on " << num_threads << " threads\n";
    Executor<LineItem> executor{num_threads};

    auto const [tasks,depths] = [argc, argv] () {
        std::vector<std::size_t> default_depths{8};
        if (argc == 1) {
            std::cerr << "No filename provided, using calgary\n";
            return std::make_pair(get_calgary_tasks(), default_depths);
        } else if (argc == 2) {
            auto const str = std::string(argv[1]);
            char const last_char = argv[1][str.size()-1];
            auto const upto = str.substr(0, str.size()-1);
            if (last_char == '_') {
                std::vector<std::size_t> all_depths(8, 0);
                std::iota(all_depths.begin(), all_depths.end(), 1);
                std::cout << "HERE: "  << all_depths.size() << " " << all_depths.front() << " " << all_depths.back() << std::endl;
                return std::make_pair(name_to_task_map.at(upto)(), all_depths);
            }
            return std::make_pair(name_to_task_map.at(str)(), default_depths);
        }  else {
            std::cerr << "Expected one file name\n";
            exit(1);
        }
    }();
    std::cout << LineItem::header() << std::endl;

    for (auto const &depth: depths) {
    for (auto const &task: tasks) {
        auto wrap_task = [task, &executor](auto ctor){executor.push_back([task, ctor](){return run_task(task, ctor);});};
        // Convinience macros for more ergonomic typing. Could do XMacro, but it's nice to have comment-out-able list
// #define pw(str, mod) wrap_task(plain_factory<mod>(str))
// #define tw(str, mod) wrap_task(tab_factory<mod>(str, log_tab_size))
#define STRINGIFY(x) #x
#define plain(str) wrap_task(plain_factory<str##Model>(depth, #str))
#define hash(str) wrap_task(tab_factory<Hash##str##Model>(\
                                depth,                    \
                                STRINGIFY(Hash##str),   \
                                1UL << log_tab_size))
#define fnvhash(str) wrap_task(tab_factory<FNVHash##str##Model>(\
                                   depth,                       \
                                   STRINGIFY(FNVHash##str),     \
                                   1UL << log_tab_size))
#define amnesia(str) wrap_task(tab_factory<Amnesia##str##Model>(\
                                   depth,                       \
                                   STRINGIFY(Amnesia##str),     \
                                   1UL <<log_tab_size))
#define pure0(str) wrap_task(tab_factory<Pure0Hash##str##Model>(    \
                                 depth,                             \
                                 STRINGIFY(Pure0Hash##str##Model),  \
                                 1UL<<log_tab_size))
#define pure1(str) wrap_task(tab_factory<Pure1Hash##str##Model>(    \
                                 depth,                             \
                                 STRINGIFY(Pure1Hash##str##Model),  \
                                 1UL<<log_tab_size))
#define pure2(str) wrap_task(tab_factory<Pure2Hash##str##Model>(    \
                                 depth,                             \
                                 STRINGIFY(Pure2Hash##str),  \
                                 1UL<<log_tab_size))
        plain(CTW);
        plain(SM1PF);
        plain(SMUKN);
        plain(PPMDP);
        plain(PPMDPFull);
        for (int log_tab_size = 7; log_tab_size < 21; ++log_tab_size) {
            hash(CTW);
            hash(SM1PF);
            hash(SMUKN);
            hash(PPMDP);
            hash(PPMDPFull);
            fnvhash(CTW);
            fnvhash(SM1PF);
            fnvhash(SMUKN);
            fnvhash(PPMDP);
            fnvhash(PPMDPFull);
            amnesia(CTW);
            amnesia(SM1PF);
            amnesia(SMUKN);
            amnesia(PPMDP);
            amnesia(PPMDPFull);
            pure0(CTW);
            pure0(SM1PF);
            pure0(SMUKN);
            pure0(PPMDP);
            pure0(PPMDPFull);
            if (log_tab_size > 8) {
                pure1(CTW);
                pure1(SM1PF);
                pure1(SMUKN);
                pure1(PPMDP);
                pure1(PPMDPFull);
            }
            if (log_tab_size > 16) {
                pure2(CTW);
                pure2(SM1PF);
                pure2(SMUKN);
                pure2(PPMDP);
                pure2(PPMDPFull);
            }
        }
#undef pure2
#undef pure1
#undef pure0
#undef amnesia
#undef fnvhash
#undef hash
#undef plain
#undef STRINGIFY
    }
    }
    auto futvec = executor.queue_work();
    for (auto &f: futvec) {
        f.wait();
        std::cout << f.get().line() << std::endl;
    }
}
