#pragma once
// Adapted from https://github.com/omerktz/VMMPredictor/blob/master/vmms/vmm/vmm/algs/ctw/VolfNode.java
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>
#include <unordered_map>
#include "model_ctx.hpp"
#include "model_mem.hpp"
#include "model_counter.hpp"
//template <typename> struct Typedebug;
constexpr double G_ALPHA{15.0};
template <std::size_t num_children>
class VolfHistogram {
public:
    constexpr static std::size_t size = num_children;
private:
    using ProbAr = std::array<prob_t, num_children>;
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
        ProbAr ret{};
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
    ProbAr learn(idx_t sym, std::size_t /*depth*/) {
        // Called in the leaf node case
        auto ret = get_probs();
        update_counts(sym);
        return ret;
    }
    ProbAr learn(idx_t sym, ProbAr const& child_probs, std::size_t depth) {
        // Internal node case
        auto const child_pw = child_probs[sym];
        auto ret = transform_probs(child_probs, depth);
        update_beta(m_counter[sym], child_pw);
        update_counts(sym);
        return ret;
    }
    ProbAr transform_probs(ProbAr const &child_probs, std::size_t /*depth*/) const {
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

template <std::size_t num_children>
class SADCTWHistogram {
public:
    constexpr static std::size_t size = num_children;
private:
    using ProbAr = std::array<prob_t, num_children>;
    //using count_t = std::size_t;
    using count_t = uint8_t;
    RescalingCounter<count_t, num_children> m_counter{};
    void update_counts(idx_t sym) {
        m_counter.increment(sym);
    }
public:
    static constexpr ProbAr get_prior() {
        ProbAr ret{};
        std::fill(ret.begin(), ret.end(), 1.0/size);
        return ret;
    }
    ProbAr get_probs() const {
        ProbAr tmp;
        // SAD estimator
        auto const sad = [this]() {
            auto n = static_cast<prob_t>(m_counter.unique());
            auto m = static_cast<prob_t>(m_counter.total());
            auto ln = std::log(n+1);
            auto lm = std::log(m);
            return m / (2*(ln-lm));
        }();
        auto const alpha = sad == 0 ? 1.0/static_cast<prob_t>(num_children) : sad;
        auto sum = static_cast<prob_t>(0.0);
        std::transform(m_counter.begin(), m_counter.end(), tmp.begin(),
                       [&sum, alpha](auto const &count) {
                           auto res = count+alpha;
                           sum += res;
                           return res;
                       });
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                       [sum](auto const &el) {
                           return el / sum;
                       });
        return tmp;
    }
    ProbAr learn(idx_t sym, std::size_t /*depth*/) {
        // Called in the leaf node case
        auto ret = get_probs();
        update_counts(sym);
        return ret;
    }
    ProbAr learn(idx_t sym, ProbAr const& child_probs, std::size_t /*depth*/) {
        // Internal node case
        // auto const child_pw = child_probs[sym];
        // auto ret = transform_probs(child_probs, depth);
        // update_counts(sym);
        // return ret;
        update_counts(sym);
        return child_probs;
    }
    ProbAr transform_probs(ProbAr const &child_probs, std::size_t /*depth*/) const {
        // auto sum = static_cast<prob_t>(0.0);
        auto my_probs = get_probs();
        auto prod = std::max(.000001, std::accumulate(child_probs.begin(), child_probs.end(), 1.0, std::multiplies<prob_t>{}));
        std::transform(my_probs.begin(), my_probs.end(), my_probs.begin(),
                       [prod](auto const &prob) {
                           return 0.5*(prob+prod);
                       });
        return my_probs;
    }
};




template <typename HistogramT>
class AmortizedVolf {
    using Node=HistogramT;
private:
    constexpr static size_t size = Node::size;
    using Ptr_t = uint32_t;
    using ProbAr = decltype(Node::get_prior());
    std::vector<Node> m_vec{};
    std::unordered_map<Ptr_t, std::array<Ptr_t, size>> m_adj{};
    std::size_t m_depth;
    std::vector<Ptr_t> get_idx_chain(IdxContext const &ctx) const {
        std::vector<Ptr_t> idxs;
        idxs.reserve(m_depth);
        idxs.push_back(0U);
        for (IdxContext c= ctx; c; c.pop()) {
            auto const parent_idx = idxs.back();
            auto idx = m_adj.at(parent_idx)[c.back()];
            if (!idx) {
                break;
            }
            idxs.push_back(idx);
        }
        return idxs;
    }
    Ptr_t get_child(Ptr_t parent_idx, size_t loc) {
        auto child_idx = m_adj.at(parent_idx)[loc];
        if (!child_idx) {
            auto new_idx = static_cast<Ptr_t>(m_vec.size());
            m_adj[parent_idx][loc] = new_idx;
            m_adj[new_idx];
            child_idx = new_idx;
            m_vec.emplace_back();
        }
        return child_idx;

    }
    std::vector<Ptr_t> make_idx_chain(IdxContext const &ctx) {
        std::vector<Ptr_t> idxs;
        idxs.reserve(m_depth+1);
        auto idx = 0U;
        idxs.push_back(idx);
        for (IdxContext c = ctx; c; c.pop()) {
            idxs.push_back(get_child(idxs.back(), c.back()));
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
        ProbAr ret{};
        if (idxs.size() <= ctx.size()+1) {
            ret = m_vec[idxs.back()].get_probs();
            idxs.pop_back();
        } else {
            ret = Node::get_prior();
        }
        auto depth = std::distance(idxs.crbegin(), idxs.crend());
        ret = std::accumulate(idxs.crbegin(), idxs.crend(), ret,
                               [this, &depth](ProbAr const& probar, auto const &idx){
                                   return m_vec[idx].transform_probs(probar, --depth);
                               });
        return ret;
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        auto depth = ctx.size()+1;
        auto idxs = make_idx_chain(ctx);
        std::accumulate(std::next(idxs.crbegin()),
                        idxs.crend(),
                        m_vec[idxs.back()].learn(sym, --depth),
                        [this, sym, &depth](ProbAr const &acc, auto const &idx) {
                            return m_vec[idx].learn(sym, acc, --depth);
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
