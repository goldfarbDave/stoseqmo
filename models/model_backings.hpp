#pragma once
template <typename HashLookupT, typename HistogramT>
class HashTableBacking {
public:
    using Node = HistogramT;
    using token_t = typename HashLookupT::hash_t;
private:

    HashLookupT m_hasher;
    std::vector<Node> m_table;
public:
    template <typename...  Args>
    HashTableBacking(std::size_t depth, size_t table_size, Args&&... args)
        : m_hasher(depth, table_size)
        , m_table(table_size, std::forward<Args>(args)...) {}
    Node &operator[](token_t const &hash) {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    Node const &operator[](token_t const &hash) const {
        return m_table[m_hasher.hash_to_idx(hash)];
    }
    std::vector<token_t> ctx_to_tokens(IdxContext const &ctx) const {
        return m_hasher.ctx_to_hashes(ctx);
    }
    // Typically, this will do a forceful insertion of new tokens, but because our hashtable is already allocated, it's simply an alias
    std::vector<token_t> make_ctx_to_tokens(IdxContext const &ctx) const {
        return ctx_to_tokens(ctx);
    }
    Footprint footprint() const {
        return Footprint{.num_nodes=m_table.size(),
                         .node_size=sizeof(Node),
                         .is_constant=true};
    }
};
template <typename HistogramT>
class AdjacencyListBacking {
public:
    using Node = HistogramT;
    using token_t = uint32_t;
private:
    using idx_t = typename IdxContext::T;
    static constexpr std::size_t size = HistogramT::size;
    std::vector<Node> m_vec{};
    std::unordered_map<token_t, std::array<token_t, size>> m_adj{};
    HistogramT m_histogram_to_copy;
    void add_histogram() {
        m_vec.push_back(m_histogram_to_copy);
    }
    token_t get_child(token_t parent_idx, idx_t loc) const {
        return m_adj.at(parent_idx)[loc];
    }
    token_t maybe_make_child(token_t parent_idx, idx_t loc) {
        auto child_idx = get_child(parent_idx, loc);
        if (!child_idx) {
            auto new_idx = static_cast<token_t>(m_vec.size());
            m_adj[parent_idx][loc] = new_idx;
            m_adj[new_idx];
            child_idx = new_idx;
            add_histogram();
        }
        return child_idx;
    }
public:
    template <typename... Args>
    AdjacencyListBacking(std::size_t /*depth*/, Args&&... args) : m_histogram_to_copy(std::forward<Args>(args)...) {
        m_vec.reserve(1<<15);
        add_histogram();
        m_adj.reserve(1<<15);
        m_adj[0];
    }
    Node &operator[](token_t const &idx) {
        return m_vec[idx];
    }
    Node const &operator[](token_t const &idx) const {
        return m_vec[idx];
    }
    std::vector<token_t> ctx_to_tokens(IdxContext const &ctx) const {
        auto parent_idx = 0U;
        std::vector<token_t> ret;
        ret.reserve(ctx.size() + 1);
        ret.push_back(parent_idx);
        for (IdxContext c = ctx; c; c.pop()) {
            auto const idx = get_child(parent_idx, c.back());
            if (!idx) break;
            ret.push_back(idx);
            parent_idx = idx;
        }
        return ret;
    }
    std::vector<token_t> make_ctx_to_tokens(IdxContext const &ctx) {
        auto parent_idx = 0U;
        std::vector<token_t> ret;
        ret.reserve(ctx.size() + 1);
        ret.push_back(parent_idx);
        for (IdxContext c = ctx; c; c.pop()) {
            ret.push_back(maybe_make_child(ret.back(), c.back()));
        }
        return ret;
    }
    Footprint footprint() const {
        return Footprint{.num_nodes=m_vec.size(),
            .node_size=sizeof(Node) + sizeof(typename decltype(m_adj)::mapped_type),
                         .is_constant=false};
    }


};
