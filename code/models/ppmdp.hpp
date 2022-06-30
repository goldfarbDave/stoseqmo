#pragma once
// Follows description in Steinruecken et al. 2015 and parameters found in ppm-dp repo
#include "ppmdp_params.hpp"
#include "ppm_template_params.hpp"
#include "data_types.hpp"


template <std::size_t num_children>
class PPMDPHistogram {
public:
    static constexpr std::size_t size = num_children;
    using count_t = std::uint8_t;
    using ProbAr = std::array<prob_t, num_children>;
    struct LearnResults {
        ProbAr probs;
        bool first_time_seen;
    };
private:
    RescalingCounter<count_t, num_children> m_multiset{};
    // Tracked externally along with depth
public:
    ProbAr transform_probs(ProbAr const &child_probs, std::size_t depth) const {
        // See Eq4.
        auto const Us = m_multiset.unique();
        auto blendparams = get_blend_params(BlendParamsIn{.depth=depth,.fanout=Us});
        auto const alpha = blendparams.alpha;
        auto const beta = blendparams.beta;
        // auto nx = m_multiset[G_c];
        // std::cout << "Count: " << int(nx)
        //           << " D: " << depth
        //           << " u: " << Us
        //           << " alpha: " << alpha
        //           << " beta: " << beta << "\n";
        // System.out.println("X: " + x +  " u: " +  u +" alpha: " + alpha + " beta: " + beta);
        // (void)depth;
        // auto const alpha = .5f;
        // auto const beta = .75f;

        ProbAr tmp;
        auto acc = static_cast<prob_t>(0.0);
        // Note, we exclude the denominator because it's cheaper to normalize at the end in an additional pass
        auto const ms = static_cast<prob_t>(m_multiset.total());
        std::transform(child_probs.cbegin(), child_probs.cend(), m_multiset.begin(), tmp.begin(),
                       [&acc, alpha, beta, Us, ms](auto const &p_c, auto const &count) {
                           auto const indic = count - beta;
                           auto const alw_term = (Us * beta + alpha)*p_c;
                           auto const ret = (indic*(count > 0) + alw_term);
                           acc += ret;
                           return ret;
                       });
        // Normalizing pass:
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), [&acc](auto const &p) {
            return p/acc;
        });
        return tmp;
    }
    static constexpr ProbAr get_prior() {
        ProbAr ret{};
        std::fill(ret.begin(), ret.end(), 1.0/num_children);
        return ret;
    }
    LearnResults learn(idx_t sym, ProbAr const &child_probs, std::size_t depth) {
        (void)depth;
        // For now we do nothing here, but if we were doing dynamic updates, we'd do them here
        // std::cout << "Learn: " << int(m_multiset[sym]) << "->" << int(m_multiset[sym]+1) << " (depth "<< depth <<")\n";
        auto const novel = !m_multiset[sym];
        m_multiset.increment(sym);
        return LearnResults{.probs=child_probs, .first_time_seen=novel};
    }
};


template <typename HistogramT, PPMUpdatePolicy update_policy>
class ExactProbDownLearnUp {
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
    Ptr_t get_child(Ptr_t parent_idx, size_t loc) const {
        return m_adj.at(parent_idx)[loc];
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
    ExactProbDownLearnUp(std::size_t depth) : m_depth{depth} {
        m_vec.reserve(1<<15);
        m_vec.emplace_back();
        m_adj.reserve(1<<15);
        m_adj[0];
    }
    ProbAr get_probs(IdxContext const &ctx) const {
        auto ret = Node::get_prior();
        auto depth = 0;
        auto idx = 0Ul;
        for (IdxContext c = ctx; c; c.pop(), depth++) {
            ret = m_vec[idx].transform_probs(ret, depth);
            idx = get_child(idx, c.back());
            if (!idx) {
                break;
            }
        }
        // auto const prior=Node::get_prior();
        // std::transform(ret.begin(),ret.end(), prior.begin(), ret.begin(),
        //                [](auto const &r, auto const &p) {
        //                    return (99*r + p)/100;
        //                });
        return ret;
    }
    void learn(IdxContext const &ctx, idx_t sym) {
        auto depth = ctx.size();
        auto idxs = make_idx_chain(ctx);
        auto probs = Node::get_prior();
        // Work backwards, updating along:
        for (auto itr = idxs.crbegin(); itr != idxs.crend(); ++itr, --depth) {
            auto res = m_vec[*itr].learn(sym, probs, depth);
            probs = res.probs;
            if constexpr (update_policy == PPMUpdatePolicy::ShallowUpdates) {
                if (!res.first_time_seen) {
                    break;
                }
            } else if constexpr (update_policy == PPMUpdatePolicy::FullUpdates) {
                continue;
            } else {
                // Uncovered update policy (ie, might want to add stochastic updates akin to 1PF)
                assert(false);
            }
        }
    }

    Footprint footprint() const {
        return {
            .num_nodes=m_vec.size(),
            .node_size=sizeof(Node) + sizeof(typename decltype(m_adj)::mapped_type),
            .is_constant=false
        };
    }
};
