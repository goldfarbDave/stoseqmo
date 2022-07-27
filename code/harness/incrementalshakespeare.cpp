#include <iostream>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "models.hpp"
#include "threadpool.hpp"

#include "file_utils.hpp"

int main() {
    // limit_gb(5);
    int const num_threads = std::thread::hardware_concurrency();
    std::cerr << "Executing on " << num_threads << " threads\n";
    Executor<IncrementalItem> executor{num_threads};
    auto task = get_shakespeare_task();
    std::cout << IncrementalItem::header() << std::endl;
    auto wrap_task = [task, &executor](auto ctor){
        executor.push_back([task, ctor](){return run_incremental_task(task, ctor);});
    };
        // Convinience macros for more ergonomic typing. Could do XMacro, but it's nice to have comment-out-able list
// #define pw(str, mod) wrap_task(plain_factory<mod>(str))
// #define tw(str, mod) wrap_task(tab_factory<mod>(str, log_tab_size))
#define STRINGIFY(x) #x
#define plain(str) wrap_task(plain_factory<str##Model>(#str))
#define hash(str) wrap_task(tab_factory<Hash##str##Model>(\
                                STRINGIFY(Hash##str),     \
                                1UL << log_tab_size))
#define amnesia(str) wrap_task(tab_factory<Amnesia##str##Model>(\
                                   STRINGIFY(Amnesia##str),     \
                                   1UL <<log_tab_size))

    plain(CTW);
    plain(SM1PF);
    plain(SMUKN);
    plain(PPMDP);
    plain(PPMDPFull);
    for (int log_tab_size = 7; log_tab_size < 20; ++log_tab_size) {
        hash(CTW);
        hash(SM1PF);
        hash(SMUKN);
        hash(PPMDP);
        hash(PPMDPFull);
        amnesia(CTW);
        amnesia(SM1PF);
        amnesia(SMUKN);
        amnesia(PPMDP);
        amnesia(PPMDPFull);
    }
#undef amnesia
#undef hash
#undef plain
#undef STRINGIFY
    auto futvec = executor.queue_work();
    for (auto &f: futvec) {
        f.wait();
        std::cout << f.get().line() << std::endl;
    }
}
