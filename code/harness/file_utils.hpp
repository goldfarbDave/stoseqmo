#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <utility>
#include "model_mem.hpp"
#include "data_types.hpp"
// Utilities for running file benchmarks and reporting results
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
struct IncrementalItem {
    std::string fn;
    std::string mn;
    std::size_t fs;
    Footprint ms;
    std::vector<prob_t> logprob_vec;
    double entropy;
    static std::string header() {
        return "File,Byte,Meth,FSize,MSize,Entropy";
    }
    std::string preamble() const {
        std::ostringstream ss;
        ss << fn << "," << mn << "," << fs << "," << ms.num_nodes << ","
           << std::setprecision(7)
           << entropy;
        return ss.str();
    }
    std::string report() const {
        std::ostringstream ss;
        ss << "START\n" << std::setprecision(7);
        for (auto const &logprob: logprob_vec) {
            ss << logprob << "\n";
        }
        ss << "END";
        return ss.str();
    }
    std::string line() const {
        return preamble() + "\n" + report();
    }
};
struct Task {
    std::string file_name;
    std::vector<byte_t> file_bytes;
};


template <typename ModelCtorT>
LineItem run_task(Task task, ModelCtorT ctor) {
    auto [model_name, model] = ctor();
    auto res = entropy_of_model(task.file_bytes, std::move(model));
    return LineItem{.fn=task.file_name,
                    .mn=model_name,
                    .fs=task.file_bytes.size(),
                    .ms=res.model.footprint(),
                    .entropy=res.H};
}

template <typename ModelCtorT>
IncrementalItem run_incremental_task(Task task, ModelCtorT ctor) {
    auto [model_name, model] = ctor();
    auto res = entropy_of_model_incremental(task.file_bytes, std::move(model));
    return IncrementalItem{.fn=task.file_name,
                           .mn=model_name,
                           .fs=task.file_bytes.size(),
                           .ms=res.model.footprint(),
                           .logprob_vec=res.logprob_vec,
                           .entropy=res.entropy};
}



template <typename RetT>
struct Executor {
    Executor(int num_threads) :m_tp{num_threads} {}
    template <typename FuncT>
    void push_back(FuncT&& task) {
        m_taskvec.emplace_back(std::forward<FuncT>(task));
    }
    std::vector<std::future<RetT>> queue_work() {
        std::vector<std::future<RetT>> futvec;
        for (auto &st: m_taskvec) {
            futvec.push_back(m_tp.add_task<RetT>(st));
        }
        return futvec;
    }
private:
    Threadpool m_tp;
    std::vector<std::packaged_task<RetT()>> m_taskvec{};
};


template <typename ListT, typename MapT>
std::vector<Task> get_corpus_tasks(ListT const& names, MapT const& name_to_path) {
    std::vector<Task> tasks;
    std::transform(names.begin(), names.end(),
                   std::back_inserter(tasks),
                   [&name_to_path](auto const &name) {
                       auto contents = load_file_in_memory(name_to_path.at(name));
                       return Task{.file_name=name,
                                   .file_bytes=contents.bytes};
                   });
    return tasks;
}
std::vector<Task> get_calgary_tasks() {
    return get_corpus_tasks(calgary_names, calgary_name_to_path);
}
Task get_shakespeare_task() {
    return Task{.file_name="shakespeare",
                .file_bytes=load_shakespeare().bytes};
}
std::vector<Task> get_shakespeare_tasks() {
    std::vector<Task> tasks;
    tasks.push_back(get_shakespeare_task());
    return tasks;
}
std::vector<Task> get_cantbry_tasks() {
    return get_corpus_tasks(cantbry_names, cantbry_name_to_path);
}
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
