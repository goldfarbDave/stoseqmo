#pragma once
// Template wrapper for adding explicit-depth context management to models
#include <type_traits>
#include "model_mem.hpp"
#include "model_ctx.hpp"

// template <typename T, typename =void>
// struct has_counter : std::false_type {};
// template <typename T>
// struct has_counter<T, std::void_t<decltype(&T::counter)>> : std::true_type {};
// template <typename T>
// inline constexpr bool has_counter_v = has_counter<T>::value;
// template <typename T, typename =void>
// struct has_footprint : std::false_type {};
// template <typename T>
// struct has_footprint<T, std::void_t<decltype(std::declval<T>().footprint())>> : std::true_type {};
// template <typename T>
// inline constexpr bool has_footprint_v = has_footprint<T>::value;

// For failing at instantiation-time rather than template-parse time
// template<class T> struct delayed_false : std::false_type {};

template <class ModelT>
class SequenceModel {
public:
    using Alphabet = typename ModelT::Alphabet;
protected:
    using sym_t = typename Alphabet::sym_t;
    MemoryDeque<idx_t> m_past_idxs;
    ModelT m_underlying;
public:
    template <typename... Args>
    SequenceModel(std::size_t depth, Args&&... args)
        : m_past_idxs{depth}
        , m_underlying{depth, std::forward<Args>(args)...} {}
    auto get_probs() const {
        return m_underlying.get_probs(m_past_idxs.view());
    }
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_underlying.learn(view, idx);
        m_past_idxs.push_back(idx);
    }
    Footprint footprint() const {
        return m_underlying.footprint();
    }
};

template <class ModelT>
class AmnesiaSequenceModel {
public:
    using Alphabet = typename ModelT::Alphabet;
private:
    using sym_t = typename Alphabet::sym_t;
    MemoryDeque<idx_t> m_past_idxs;
    ModelT m_underlying;
    std::size_t m_max_nodes;
    std::size_t m_depth;
public:
    AmnesiaSequenceModel(std::size_t depth, std::size_t max_nodes)
        : m_past_idxs{depth}
        , m_underlying{depth}
        , m_max_nodes{max_nodes}
        , m_depth{depth} {}
    auto get_probs() const {
        return m_underlying.get_probs(m_past_idxs.view());
    }
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_underlying.learn(view, idx);
        // Forget! Note that we keep our old context
        if (m_underlying.footprint().num_nodes > m_max_nodes) {
            m_underlying = ModelT{m_depth};
            m_underlying.learn(view, idx);
        }
        m_past_idxs.push_back(idx);
    }
    Footprint footprint() const {
        return {.num_nodes = m_max_nodes,
                .node_size=this->m_underlying.footprint().node_size,
                .is_constant=true};
    }
};
