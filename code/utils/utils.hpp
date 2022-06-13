#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <iostream> // Remove once reworked timesection
class TimeSection {
    using time_t = decltype(std::chrono::high_resolution_clock::now());
    std::string name;
    time_t start;
public:
    TimeSection(std::string name_)
        : name(name_)
        , start{std::chrono::high_resolution_clock::now()} {}
    ~TimeSection() {
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
        std::cout << name << " took " << ms << "ms (" << ns << "us)" << std::endl;
    }
};
template <typename DataT>
struct Context {
    // End points one past the back
    using ItrT = typename std::vector<DataT>::const_iterator;
    ItrT m_start, m_end;
    Context(ItrT start_, ItrT end_) : m_start{start_}, m_end{end_} {
        assert(m_start <= m_end);
    }
    // For use in bool contexts
    explicit operator bool() const {
        return m_start!=m_end;
    }
    ItrT begin() const {
        return m_start;
    }
    ItrT end() const {
        return m_end;
    }
    ItrT rbegin() const {
        return m_end - 1;
    }
    ItrT rend() const {
        return m_start - 1;
    }
    DataT back() const {
        return *(m_end-1);
    }
    DataT pop() {
        assert(m_start != m_end);
        return *--m_end;
    }
    Context<DataT> popped() const {
        assert(m_start != m_end);
        return Context(m_start, m_end-1);
    }
};

template <typename T>
class MemoryDeque {
    std::vector<T> data;
    std::size_t window_size;
public:
    MemoryDeque(std::size_t size) : window_size{size} {}
    void push_back(T item) {
        data.push_back(item);
    }
    Context<T> view() const {
        // Unfull case:
        if (data.size() < window_size) {
            return Context<T>(data.begin(), data.end());
        }
        return Context<T>(data.end()-window_size, data.end());
    }
    void clear() {
        data.clear();
    }
};

template <typename ModelT, typename sym_t>
double entropy_of_model(std::vector<sym_t> const &syms, ModelT model) {
    double H = 0.0;
    for (auto const & sym: syms) {
        H += -log2l(model.pmf(sym));
        model.learn(sym);
    }
    //std::cout << make_size_string(model);
    return H;
}
struct Footprint {
    std::size_t num_nodes;
    std::size_t node_size;
};

template <typename ModelT>
std::string make_size_string(ModelT const &model) {
    Footprint f = model.footprint();
    auto num_nodes = f.num_nodes;
    auto node_size = f.node_size;
    auto mib=((num_nodes*node_size) >> 20);
    auto kib=((num_nodes*node_size) >> 10);
    std::stringstream ss;
    ss << "Number of nodes created: " << num_nodes << " each of size " << node_size << "B for a total of " << mib << "MiB (" << kib << " KiB)" << "\n";
    return ss.str();
}
