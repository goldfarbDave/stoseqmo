#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>
#include <iomanip>
#include "corpus.hpp"
#include "utils.hpp"


//template <typename> struct Typedebug;
template <std::size_t num_children>
class VolfNode {
    using idx_t = size_t;
    using IdxContext = Context<idx_t>;
    using ProbAr = std::array<double, num_children>;
    using count_t = uint8_t;
    // using count_t = std::size_t;
    std::array<std::unique_ptr<VolfNode>, num_children> m_children{};
    std::array<count_t, num_children> m_counts{};
    std::size_t m_total{};
    double m_beta{1.0};
    double m_alpha;
    std::unique_ptr<VolfNode> make_child() {
        return std::make_unique<VolfNode>(m_alpha);
    }
    VolfNode& get_child(idx_t idx) {
        if (!m_children[idx]) {
            m_children[idx] = make_child();
        }
        return *m_children[idx];
    }
    VolfNode const& get_child(idx_t idx) const {
        assert(m_children[idx]);
        return *m_children[idx];
    }
    ProbAr get_theoretical_new_child_pe_ar() const {
        ProbAr tmp;
        std::fill(tmp.begin(), tmp.end(), 1.0/num_children);
        return tmp;
    }
    ProbAr get_pe_ar() const {
        ProbAr tmp;
        std::transform(m_counts.begin(), m_counts.end(), tmp.begin(),
                       [this](auto const& el) {
                           return (el + (1/m_alpha))
                               / (m_total + (num_children/m_alpha));
                       });
        return tmp;
    }
    void theoretical_apply_weighting(ProbAr &prob_ar) const {
        auto sum = 0.0;
        std::transform(prob_ar.begin(), prob_ar.end(), prob_ar.begin(),
                       [this,&sum] (auto const &prob) {
                           auto val = prob + (1.0/num_children);
                           sum += val;
                           return val;
                       });
        std::transform(prob_ar.begin(), prob_ar.end(), prob_ar.begin(),
                       [sum] (auto const &prob) {
                           return prob/sum;
                       });
    }
    void apply_weighting(ProbAr &prob_ar) const {
        auto sum = 0.0;
        std::transform(prob_ar.begin(), prob_ar.end(), m_counts.begin(), prob_ar.begin(),
                       [&sum, this](auto const& prob, auto const& count) {
                           auto val =prob + (count + 1/m_alpha)*beta_tag();
                           sum += val;
                           return val;
                       });
        // auto sum = std::accumulate(prob_ar.begin(), prob_ar.end(), 0.0);
        std::transform(prob_ar.begin(), prob_ar.end(), prob_ar.begin(),
                       [sum](auto const &prob) {
                           return prob/sum;
                       });
    }

    auto beta_tag() const {
        return m_beta / (num_children/m_alpha + m_total);
    }
    void update_beta(count_t count, double child_pw) {
        // Update beta, consts from thesis
        auto const beta_thresh = 1500000;
        if ((m_beta > beta_thresh) || (m_beta < (1.0/beta_thresh))) {
            m_beta /= 2;
        } else {
            m_beta = (count + (1/m_alpha))*beta_tag()/child_pw;
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
    static std::size_t num_created;
    VolfNode(double alpha) : m_alpha{alpha} {++num_created;}
    ProbAr get_probs(IdxContext const &ctx) const {
        if (!ctx) {
            return get_pe_ar();
        }
        auto idx = ctx.back();
        ProbAr child_probs;
        if (m_children[idx]) {
            auto &child = get_child(idx);
            child_probs = child.get_probs(ctx.popped());
        } else {
            // Unroll children to maintain const-ness
            child_probs = get_theoretical_new_child_pe_ar();
            for (auto ictx = ctx; ictx; ictx=ictx.popped()) {
                theoretical_apply_weighting(child_probs);
            }
        }
        apply_weighting(child_probs);
        return child_probs;
    }
    ProbAr learn(IdxContext const& ctx, idx_t sym) {
        ProbAr pv;
        if (ctx) {
            auto idx = ctx.back();
            auto &child = get_child(idx);
            pv = child.learn(ctx.popped(), sym);
            auto child_pw = pv[sym];
            apply_weighting(pv);
            update_beta(m_counts[sym], child_pw);
        } else {
            pv = get_pe_ar();
        }
        update_counts(sym);
        return pv;
    }
};
// Give space for statics here.
template<std::size_t N>
std::size_t VolfNode<N>::num_created;


template <typename Alphabet>
class VolfModel {
    // Standard K-Ary implementation discussed as "Solution 1"
    using idx_t = std::size_t;

    MemoryDeque<idx_t> m_past_idxs;
    VolfNode<Alphabet::size> m_root;
    auto get_probs() const{
        return m_root.get_probs(m_past_idxs.view());
    }
public:
    using sym_t = typename Alphabet::dtype;
    VolfModel(size_t depth, double alpha)
        : m_past_idxs{depth}
        , m_root{alpha}
        {
            VolfNode<Alphabet::size>::num_created = 0;
        }
    Footprint footprint() const {
        return {.num_nodes = VolfNode<Alphabet::size>::num_created,
                .node_size = sizeof(VolfNode<Alphabet::size>)};
    }
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_root.learn(view, idx);
        m_past_idxs.push_back(idx);
    }
    double pmf(sym_t sym) const {
        auto idx = Alphabet::to_idx(sym);
        return get_probs()[idx];
    }
    double excmf(sym_t sym) const {
        auto probar = get_probs();
        auto cmf = std::accumulate(probar.begin(),
                                   probar.begin()+Alphabet::to_idx(sym),
                                   0.0);
        return cmf;
    }
    sym_t find_sym_from_cum_prob(double cum_prob) const {
        auto probar = get_probs();
        auto pos = std::find_if(probar.begin(), probar.end(),
                                [&cum_prob](auto const& prob) {
                                    if (cum_prob < prob) {
                                        return true;
                                    }
                                    cum_prob -= prob;
                                    return false;
                                });
        return Alphabet::to_sym(std::distance(probar.begin(), pos));
    }
};

#include <sys/resource.h>
#include "ac.hpp"
int main() {
    // 3GB limit
    auto tgb = (1ul<<30)*3;
    struct rlimit limit{tgb, tgb};
    if (-1 == setrlimit(RLIMIT_DATA, &limit)) {
        std::cerr << "Failed to set mem safety" << "\n";
        return -1;
    }
    auto dmax{9};
    correctness_and_entropy_test([]() {
        return VolfModel<BitAlphabet>(10, 15.0);
    });
    // for (auto const &name: calgary_names) {
    //     auto contents = load_file_in_memory(calgary_name_to_path.at(name));
    //     std::cout << name << " ";
    // }
    // std::cout << "File,Alphabet,Depth,Entropy" << std::endl;
    // for (auto const & name: calgary_names) {
    //     auto const path = calgary_name_to_path.at(name);
    //     auto contents = load_file_in_memory(path);
    //     for (int depth = 1; depth < dmax; ++depth) {
    //         std::cout << name << ",Bit," << depth << ","
    //                   << std::setprecision(7)
    //                   << entropy_of_model(contents.bits, VolfModel<BitAlphabet>(depth, 15.0))
    //                   << std::endl;
    //     }
    //     for (int depth = 1; depth < dmax; ++depth) {
    //         std::cout << name << ",Byte," << depth << ","
    //                   << std::setprecision(7)
    //                   << entropy_of_model(contents.bytes, VolfModel<ByteAlphabet>(depth, 15.0))
    //                   << std::endl;
    //     }
    // }

}
