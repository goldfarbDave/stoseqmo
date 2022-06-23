// Utilities for context-based models.
#pragma once
#include <cassert>
#include <vector>

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
    std::size_t size() const {
        return std::distance(m_start, m_end);
    }
    Context<DataT> popped() const {
        assert(m_start != m_end);
        return Context(m_start, m_end-1);
    }
};
// using idx_t = std::size_t;
using IdxContext = Context<idx_t>;
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
