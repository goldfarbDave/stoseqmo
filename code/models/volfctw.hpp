#pragma once
// Adapted from https://github.com/omerktz/VMMPredictor/blob/master/vmms/vmm/vmm/algs/ctw/VolfNode.java
#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>

#include "model_ctx.hpp"
#include "model_mem.hpp"
#include "model_counter.hpp"
//template <typename> struct Typedebug;
constexpr double G_ALPHA{15.0};
template <std::size_t num_children>
class VolfHistogram {
private:
    constexpr static std::size_t size = num_children;
    using ProbAr = std::array<double, num_children>;
    //using count_t = std::size_t;
    using count_t = uint8_t;
    RescalingCounter<count_t, num_children> m_counter{};
    double m_beta{1.0};
    auto beta_tag() const {
        return m_beta / (static_cast<double>(num_children)/G_ALPHA
                         + static_cast<double>(m_counter.total()));
    }
public:
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
        m_counter.increment(sym);
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
        std::transform(m_counter.begin(), m_counter.end(), tmp.begin(),
                       [this](auto const& el) {
                           return (el + (1/G_ALPHA))
                               / (static_cast<double>(m_counter.total())
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
        update_beta(m_counter[sym], child_pw);
        update_counts(sym);
        return ret;
    }
    ProbAr transform_probs(ProbAr const &child_probs) const {
        ProbAr tmp;
        auto sum = 0.0;
        std::transform(child_probs.begin(), child_probs.end(), m_counter.begin(), tmp.begin(),
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

template <std::size_t N>
class AmortizedVolf {
public:
    static constexpr std::size_t size = N;
    using Node=VolfHistogram<size>;
private:
    using Ptr_t = uint32_t;
    using ProbAr = std::array<double, size>;
    std::vector<Node> m_vec{};
    std::unordered_map<Ptr_t, std::array<Ptr_t, size>> m_adj{};
    std::size_t m_depth;
    std::vector<Ptr_t> get_idx_chain(IdxContext const &ctx) const {
        std::vector<Ptr_t> idxs;
        idxs.reserve(m_depth);
        auto idx = 0U;
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
        auto idx = 0U;
        idxs.push_back(idx);
        for (IdxContext c = ctx; c; c.pop()) {
            auto child_idx = m_adj.at(idx)[c.back()];
            if (!child_idx) {
                auto new_idx = static_cast<Ptr_t>(m_vec.size());
                m_adj[idx][c.back()] = new_idx;
                m_adj[new_idx];
                child_idx = new_idx;
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
