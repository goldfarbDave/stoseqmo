#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <utility>
#include "model_mem.hpp"
#include "strong_types.hpp"
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

template<template<typename AlphabetUsed> class ModelT>
auto plain_factory(std::string const &str) {
    return [str](){return std::make_pair(str, ModelT<ByteAlphabet>(DEPTH));};
}
template<template<typename AlphabetUsed> class ModelT>
auto tab_factory(std::string const &str, int log_tab_size) {
    return [str, log_tab_size](){
        return std::make_pair(str,
                              ModelT<ByteAlphabet>(DEPTH, 1UL<<log_tab_size));};
}

std::vector<Task> get_calgary_tasks() {
    std::vector<Task> tasks;
    std::transform(calgary_names.begin(), calgary_names.end(),
                   std::back_inserter(tasks),
                   [](auto const &name) {
                       auto contents = load_file_in_memory(calgary_name_to_path.at(name));
                       return Task{.file_name=name,
                                   .file_bytes=contents.bytes};
                   });
    return tasks;
}
template <typename ListT, typename MapT>
std::vector<Task> get_corpus_tasks(ListT const& names, MapT const& name_to_path) {
    std::vector<Task> tasks;
    std::transform(names.begin(), names.end(),
                   std::back_inserter(tasks),
                   [](auto const &name) {
                       auto contents = load_file_in_memory(name_to_path.at(name));
                       return Task{.file_name=name,
                                   .file_bytes=contents.bytes};
                   });
    return tasks;

}
std::vector<Task> get_calgary_tasks() {
    return get_corpus_tasks(calgary_names, calgary_name_to_path);
}
std::vector<Task> get_shakespeare_tasks() {
    std::vector<Task> tasks;
    tasks.push_back(Task{.file_name="shakespeare",
                         .file_bytes=load_shakespeare().bytes});
    return tasks;
}
std::vector<Task> get_cantbry_tasks() {
    return get_corpus_tasks(cantbry_names, cantbry_name_to_path);
}
