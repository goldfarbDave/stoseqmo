// Simple interface to allow a model to communicate its memory footprint
#pragma once
#include <sstream>
#include <string>

struct Footprint {
    std::size_t num_nodes;
    std::size_t node_size;
    bool is_constant;

    auto mib() const {
        return ((num_nodes*node_size) >> 20);
    }
    auto kib() const {
        return ((num_nodes*node_size) >> 10);
    }
};

template <typename ModelT>
std::string make_size_string(ModelT const &model) {
    Footprint f = model.footprint();
    auto num_nodes = f.num_nodes;
    auto node_size = f.node_size;
    std::ostringstream ss;
    ss << "Number of nodes created: " << num_nodes
       << " each of size " << node_size << "B for a total of " << f.mib() << "MiB ("
       << f.kib() << " KiB) (" << (f.is_constant ? "capped" : "uncapped") << ")"
       <<"\n";
    return ss.str();
}
