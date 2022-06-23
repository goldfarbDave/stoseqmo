// Adapted from https://github.com/omerktz/VMMPredictor/blob/master/vmms/vmm/vmm/algs/ctw/VolfNode.java
#pragma once
#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>

#include "model_ctx.hpp"
#include "model_mem.hpp"

//template <typename> struct Typedebug;
constexpr double G_ALPHA{15.0};
template <std::size_t num_children>
class VolfHistogram {
private:
    constexpr static std::size_t size = num_children;
    using ProbAr = std::array<double, num_children>;
    using count_t = uint8_t;
    // using count_t = std::size_t;
    std::size_t m_total{};
    double m_beta{1.0};
    auto beta_tag() const {
        return m_beta / (static_cast<double>(num_children)/G_ALPHA
                         + static_cast<double>(m_total));
    }
public:
    std::array<count_t, num_children> m_counts{};
    void update_beta(count_t count, double child_pw) {
        // Update beta, consts from thesis
        auto const beta_thresh = 1500000;
        if ((m_beta > beta_thresh) || (m_beta < (1.0/beta_thresh))) {
            m_beta /= 2;
        } else {
            m_beta = (count + (1/G_ALPHA))*beta_tag()/child_pw;
        }
    }
    void update_counts(idx_t sym) {
        m_counts[sym] += 1;
        m_total += 1;
        auto const RESCALE_THRESHOLD = std::numeric_limits<uint8_t>::max();
        if (m_counts[sym] == RESCALE_THRESHOLD) {
            auto new_total = 0;
            std::transform(m_counts.begin(), m_counts.end(), m_counts.begin(),
                           [this, &new_total](auto const &el) {
                               // Divide by 2, round up
                               auto nc =(el >> 1) + (el & 1);
                               new_total += nc;
                               return nc;
                           });
            m_total = new_total;
        }
    }
public:
    static constexpr ProbAr get_prior() {
        ProbAr ret;
        std::fill(ret.begin(), ret.end(), 1.0/size);
        return ret;
    }
    ProbAr get_probs() const {
        // Pe in the 95 paper
        ProbAr tmp;
        std::transform(m_counts.begin(), m_counts.end(), tmp.begin(),
                       [this](auto const& el) {
                           return (el + (1/G_ALPHA))
                               / (static_cast<double>(m_total)
                                  + static_cast<double>((num_children/G_ALPHA)));
                       });
        return tmp;
    }
    ProbAr learn(idx_t sym) {
        // Called in the leaf node case
        auto ret = get_probs();
        update_counts(sym);
        return ret;
    }
    ProbAr learn(idx_t sym, ProbAr const& child_probs) {
        // Internal node case
        auto const child_pw = child_probs[sym];
        auto ret = transform_probs(child_probs);
        update_beta(m_counts[sym], child_pw);
        update_counts(sym);
        return ret;
    }
    ProbAr transform_probs(ProbAr const &child_probs) const {
        ProbAr tmp;
        auto sum = 0.0;
        std::transform(child_probs.begin(), child_probs.end(), m_counts.begin(), tmp.begin(),
                       [&sum, this](auto const& prob, auto const& count) {
                           auto val =prob + (count + 1/G_ALPHA)*beta_tag();
                           sum += val;
                           return val;
                       });
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                       [sum](auto const &prob) {
                           return prob/sum;
                       });
        return tmp;
    }
};

template <typename AlphabetT>
class AmortizedVolf {
public:
    using Alphabet = AlphabetT;
    static constexpr std::size_t size = Alphabet::size;
    using Node=VolfHistogram<size>;
private:
    using Ptr_t = uint64_t;
    using ProbAr = std::array<double, size>;
    std::vector<Node> m_vec;
    std::unordered_map<Ptr_t, std::array<Ptr_t, size>> m_adj;
    std::size_t m_depth;
    std::vector<Ptr_t> get_idx_chain(IdxContext const &ctx) const {
        std::vector<Ptr_t> idxs;
        idxs.reserve(m_depth);
        auto idx = 0UL;
        idxs.push_back(idx);
        for (IdxContext c= ctx; c; c.pop()) {
            idx = m_adj.at(idx)[c.back()];
            if (!idx) {
                break;
            }
            idxs.push_back(idx);
        }
        return idxs;
    }

    std::vector<Ptr_t> make_idx_chain(IdxContext const &ctx) {
        std::vector<Ptr_t> idxs;
        idxs.reserve(m_depth);
        auto idx = 0UL;
        idxs.push_back(idx);
        for (IdxContext c = ctx; c; c.pop()) {
            auto child_idx = m_adj.at(idx)[c.back()];
            if (!child_idx) {
                m_adj[idx][c.back()] = m_vec.size();
                m_adj[m_vec.size()];
                child_idx = m_vec.size();
                m_vec.emplace_back();
            }
            idxs.push_back(child_idx);
            idx = child_idx;
        }
        return idxs;
    }
public:
    AmortizedVolf(std::size_t depth) : m_depth{depth} {
        m_vec.reserve(1<<15);
        m_vec.emplace_back();
        m_adj.reserve(1<<15);
        m_adj[0];
    }
    ProbAr get_probs(IdxContext const &ctx) const {
        auto idxs = get_idx_chain(ctx);
        ProbAr ret;
        if (idxs.size() <= ctx.size()+1) {
            ret = m_vec[idxs.back()].get_probs();
            idxs.pop_back();
        } else {
            ret = Node::get_prior();
        }
        return std::accumulate(idxs.crbegin(), idxs.crend(), ret,
                               [this](ProbAr const& probar, auto const &idx){
                                   return m_vec[idx].transform_probs(probar);
                               });
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        auto idxs = make_idx_chain(ctx);
        std::accumulate(std::next(idxs.crbegin()),
                        idxs.crend(),
                        m_vec[idxs.back()].learn(sym),
                        [this, sym](ProbAr const &acc, auto const &idx) {
                            return m_vec[idx].learn(sym, acc);
                        });
    }

    Footprint footprint() const {
        return {
            .num_nodes=m_vec.size(),
            .node_size=sizeof(Node) + sizeof(typename decltype(m_adj)::mapped_type),
            .is_constant=false
        };
    }
};
