#include <iostream>
#include "limit_mem.hpp"

#include "corpus.hpp"
#include "model_utils.hpp"

#include "models.hpp"
#include "threadpool.hpp"

#include "file_utils.hpp"
constexpr auto DEPTH = 8;

int main() {
    limit_gb(5);
    auto const tasks = get_calgary_tasks();
    Threadpool tp(3);

    std::vector<std::packaged_task<LineItem()>> stvec;
    std::cout << LineItem{}.header() << std::endl;
    for (auto const &task: tasks)
        auto wrap_task = [task, &stvec](auto ctor){stvec.emplace_back([task, ctor](){return do_task(task, ctor);});};
        // Convinience macros for more ergonomic typing. Could do XMacro, but it's nice to have comment-out-able list
// #define pw(str, mod) wrap_task(plain_factory<mod>(str))
// #define tw(str, mod) wrap_task(tab_factory<mod>(str, log_tab_size))
#define STRINGIFY(x) #x
#define plain_run(str) wrap_task(plain_factory<str##Model>(#str))
#define hash_run(str) wrap_task(tab_factory<Hash##str##Model>(STRINGIFY(Hash##str), log_tab_size))
#define amnesia_run(str) wrap_task(tab_factory<Amnesia##str##Model>(STRINGIFY(Amnesia##str), log_tab_size))
        plain(CTW);
        plain(SM1PF);
        plain(SMUKN);
        plain(PPMDP);
        plain(PPMDPFull);
        for (int log_tab_size = 7; log_tab_size < 8; ++log_tab_size) {
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
#undef amnesia_run
#undef hash_run
#undef plain_run
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
