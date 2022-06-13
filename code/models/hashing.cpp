#include <algorithm>
#include <iostream>
#include <vector>
#include <numeric>
#include <utility>
#include <iomanip>
#include "utils.hpp"
#include "corpus.hpp"

// Boost's impl
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
// template <typename El, typename InitFunc, typename BodyFunc>
// using NestedApply_t = decltype(declval<BodyFunc>{}(declval<InitFunc>{}(declval<El>{})));

// template <typename El, typename InitFunc, typename BodyFunc>
// NestedApply_t<C::value_type, InitFunc, BodyFunc>
// pop_accumulate(C cont, InitFunc initfunc, BodyFunc bf) {
//     auto tmp = init_func(cont.back());
//     cont.pop_back();
//     while (!cont.empty()) {
//         body_func
//     }
// }
template <typename K, typename V>
class ContextHashTable {
    // For now, let's assume we can peel size out of our value type
    static_assert(V::size);
    std::vector<V> m_table;
    using ProbAr = std::array<double, V::size>;
    V& lookup(std::size_t hash) {
        return m_table[hash % m_table.size()];
    }
    V const & lookup(std::size_t hash) const {
        return m_table[hash % m_table.size()];
    }

    std::size_t start_seed{0};
    std::vector<std::size_t> ctx_to_hashed_idxs(Context<K> ctx) const {
        std::vector<std::size_t> idxs;
        auto seed{start_seed};
        idxs.push_back(start_seed);
        while (ctx) {
            hash_combine(seed, ctx.pop());
            idxs.push_back(seed);
        }
        return idxs;
    }
public:
    ContextHashTable(std::size_t num_entries) : m_table(num_entries) {}

    void learn(Context<K> const &ctx, K const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        std::accumulate(std::next(hashed_idxs.rbegin()),
                        hashed_idxs.rend(),
                        lookup(hashed_idxs.back()).learn_base(sym),
                        [this, sym](ProbAr acc, auto const &idx) {
                            return lookup(idx).learn_non_base(sym, acc);
                        });
    }
    ProbAr get_probs(Context<K> ctx) const {
        // Let's be naive and build up our counts, then iterate backwards:
        auto hashed_idxs = ctx_to_hashed_idxs(ctx);
        return std::accumulate(std::next(hashed_idxs.rbegin()),
                               hashed_idxs.rend(),
                               lookup(hashed_idxs.back()).get_probs(),
                               [this](ProbAr acc, auto const &idx) {
                                   return lookup(idx).get_probs(acc);
                               });
        // ProbAr ret = m_table[hashed_idxs.back()].get_probs();
        // hashed_idxs.pop_back();
        // while (!hashed_idxs.empty()) {
        //     tmp = m_table[hashed_idxs.back()].get_probs(ret);
        //     vals.pop_back();
        // }
        // return ret;
    }

    std::size_t size() const {
        return m_table.size();
    }
};
constexpr double g_alpha = 15.0;
template <typename T, std::size_t N>
class Histogram {
private:
    using ProbAr = std::array<double, N>;
    ProbAr get_pe_ar() const {
        ProbAr tmp;
        std::transform(m_counts.begin(), m_counts.end(), tmp.begin(),
                       [this](auto const& el) {
                           return (el + (1/g_alpha))
                               / (m_total + (N/g_alpha));
                       });
        return tmp;
    }
    ProbAr apply_weighting(ProbAr const &prob_ar) const {
        ProbAr tmp;
        auto sum = 0.0;
        std::transform(prob_ar.begin(), prob_ar.end(), m_counts.begin(), tmp.begin(),
                       [&sum, this](auto const& prob, auto const& count) {
                           auto val =prob + (count + 1/g_alpha)*beta_tag();
                           sum += val;
                           return val;
                       });
        // auto sum = std::accumulate(prob_ar.begin(), prob_ar.end(), 0.0);
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                       [sum](auto const &prob) {
                           return prob/sum;
                       });
        return tmp;
    }
    void update_beta(T count, double child_pw) {
        // Update beta, consts from thesis
        auto const beta_thresh = 1500000;
        if ((m_beta > beta_thresh) || (m_beta < (1.0/beta_thresh))) {
            m_beta /= 2;
        } else {
            m_beta = (count + (1/g_alpha))*beta_tag()/child_pw;
        }
    }
    auto beta_tag() const {
        return m_beta / (N/g_alpha + m_total);
    }
    std::array<T, N> m_counts;
    std::size_t m_total;
    double m_beta{1.0};
    void update_counts(std::size_t idx) {
        m_counts[idx] += 1;
        m_total += 1;
        auto const RESCALE_THRESHOLD = std::numeric_limits<uint8_t>::max();
        if (m_counts[idx] == RESCALE_THRESHOLD) {
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
    static constexpr std::size_t size = N;

    Histogram() : m_total{0} {
        std::fill(m_counts.begin(), m_counts.end(), 0);
    }
    ProbAr learn(std::size_t idx, std::optional<ProbAr> const & child_probs=std::nullopt) {
        if (child_probs) {
            return learn_base(idx);
        }
        return learn_non_base(idx, *child_probs);
    }

    ProbAr learn_base(std::size_t idx) {
        auto ret = get_pe_ar();
        update_counts(idx);
        return ret;
    }
    ProbAr learn_non_base(std::size_t idx, ProbAr const & child_probs) {
        auto child_pw = child_probs[idx];
        auto ret = apply_weighting(child_probs);
        update_beta(m_counts[idx], child_pw);
        update_counts(idx);
        return ret;
    }
    ProbAr get_probs(std::optional<ProbAr> const & child_probs =std::nullopt) const {
        if (!child_probs) {
            return get_pe_ar();
        }
        return apply_weighting(*child_probs);
    }
};
// namespace std {
//     template <typename DataT>
//     struct hash<Context<DataT>> {
//         std::size_t operator()(Context<DataT> ctx) const {
//             std::size_t seed = 0;
//             while (ctx) {
//                 hash_combine(seed, ctx.back_and_pop());
//             }
//             return seed;
//         }
//     };
// }
// Hashing ideas: FNV, pearson
template <typename Alphabet>
class HashModel {
public:
    using sym_t = typename Alphabet::dtype;
private:
    using count_t = std::size_t;
    using idx_t = std::size_t;
    using histogram_t = Histogram<count_t, Alphabet::size>;
    MemoryDeque<idx_t> m_past_idxs;
    ContextHashTable<idx_t, histogram_t> m_hashtable;
    auto get_probs() const {
        return m_hashtable.get_probs(m_past_idxs.view());
    }
public:
    HashModel (std::size_t num_entries, std::size_t depth) : m_past_idxs{depth}, m_hashtable{num_entries} {}
    void learn(sym_t sym) {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
        m_hashtable.learn(view, idx);
        m_past_idxs.push_back(idx);
    }

    double pmf(sym_t sym) const {
        auto idx = Alphabet::to_idx(sym);
        auto view = m_past_idxs.view();
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
    correctness_and_entropy_test([]() {
        return HashModel<BitAlphabet>(1<<21, 10);
    });
    // auto tsmax{1<<21};
    // auto dmax{10};
    // std::cout << "File,Alphabet,TS,Depth,Entropy" << std::endl;
    // for (auto const & name: cantbry_names) {
    //     auto const path = cantbry_name_to_path.at(name);
    //     auto contents = load_file_in_memory(path);
    //     for (int ts = 128; ts < tsmax; ts <<= 1) {
    //         for (int depth = 1; depth < dmax; ++depth) {
    //             std::cout << name << ",Byte," << ts << "," << depth << ","
    //                       << std::setprecision(7)
    //                       << entropy_of_model(contents.bytes, HashModel<ByteAlphabet>(ts, depth))
    //                       << std::endl;
    //         }
    //     }
    // }
}
