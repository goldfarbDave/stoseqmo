#include <algorithm>
#include <array>
#include "corpus.hpp"
#include "utils.hpp"

template <typename Alphabet>
class Order0Model {
    using count_t = std::size_t;
    using sym_t = typename Alphabet::dtype;
    std::array<count_t, Alphabet::size> m_counts;
    std::size_t m_total;
public:
    Order0Model()
        : m_total{Alphabet::size} {
        std::fill(m_counts.begin(), m_counts.end(), 1);
    }
    Footprint footprint() const {
        return {.num_nodes=1,
                .node_size=sizeof(this)};
    }
    void learn(sym_t sym) {
        m_counts[Alphabet::to_idx(sym)] += 1;
        m_total += 1;
    }
    double pmf(sym_t sym) const {
        return static_cast<double>(m_counts[Alphabet::to_idx(sym)])/m_total;
    }
    double excmf(sym_t sym) const {
        auto counts = std::accumulate(m_counts.begin(),
                                      m_counts.begin()+Alphabet::to_idx(sym),
                                      0);
        return static_cast<double>(counts)/m_total;
    }
    sym_t find_sym_from_cum_prob(double prob) const {
        count_t cum_acc = prob*m_total;
        auto pos = std::find_if(m_counts.begin(), m_counts.end(),
                                [&cum_acc](auto const& count) {
                                    if (cum_acc < count) {
                                        return true;
                                    }
                                    cum_acc -= count;
                                    return false;
                                });
        return Alphabet::to_sym(std::distance(m_counts.begin(), pos));
    }

};
template <typename Alphabet>
class UniformModel {
    using count_t = std::size_t;
    using sym_t = typename Alphabet::dtype;
public:
    Footprint footprint() const {
        return {.num_nodes=0,
                .node_size=sizeof(this)};
    }
    void learn(sym_t sym) {
    }
    double pmf(sym_t sym) const{
        return 1.0/Alphabet::size;
    }
};

int main() {
    auto [ar, bar] = []() {
        TimeSection var("Opening File");
        // auto ar = open_file_as_bytes("../data/enwik8/enwik8");
        auto ar = open_file_as_bytes("../data/shakespeare/shakespeare");
        //auto ar = open_file_as_bytes("../data/cantrbry/sum");
        auto bar = bytevec_to_bitvec(ar);
        return std::make_pair(ar, bar);
    }();
    TimeSection var("Entropy");
    // std::cout << entropy_of_model(ar, Order0Model<ByteAlphabet>()) << std::endl;
    // std::cout << entropy_of_model(bar, Order0Model<BitAlphabet>()) << std::endl;
    // std::cout << entropy_of_model(ar, UniformModel<ByteAlphabet>()) << std::endl;
    // std::cout << entropy_of_model(bar, UniformModel<BitAlphabet>()) << std::endl;
}
