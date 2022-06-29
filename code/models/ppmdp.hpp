#pragma once
// Follows description in Steinruecken et al. 2015 and parameters found in ppm-dp repo
#include "ppmdp_params.hpp"
#include "data_types.hpp"
// Forward decl because
class CoinFlipper;

template <std::size_t num_children>
class PPMDPHistogram {
public:
    static constexpr std::size_t size = num_children;
    using count_t = std::uint8_t;
    using ProbAr = std::array<prob_t, num_children>;
private:
    RescalingCounter<count_t, num_children> m_multiset{};
    // Tracked externally along with depth
public:
    //PPMDPHistogram(CoinFlipper &ref) {(void)ref;}
    ProbAr transform_probs(ProbAr const &child_probs, std::size_t depth) const {
        // See Eq4.
        auto const Us = m_multiset.unique();
        auto blendparams = get_blend_params(BlendParamsIn{.depth=depth,.fanout=m_multiset.unique()});
        auto const alpha = blendparams.alpha;
        auto const beta = blendparams.beta;
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
    ProbAr learn(idx_t sym, std::size_t /*depth*/) {
        // For now we do nothing here, but if we were doing dynamic updates, we'd do them here
        auto prior = get_prior();
        m_multiset.increment(sym);
        return prior;
    }
    ProbAr learn(idx_t sym, ProbAr const &child_probs, std::size_t /*depth*/) {
        // For now we do nothing here, but if we were doing dynamic updates, we'd do them here
        m_multiset.increment(sym);
        return child_probs;
    }
};
