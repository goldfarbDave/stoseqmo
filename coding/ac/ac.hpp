#pragma once
#include "data_types.hpp"

constexpr std::size_t PREC{64};
constexpr std::size_t WHOLEB{PREC-2};
constexpr std::size_t Q1B{1UL << (WHOLEB - 2)};
constexpr std::size_t Q2B{1UL << (WHOLEB - 1)};
constexpr std::size_t Q4B{(1UL << WHOLEB) - 1};
constexpr std::size_t MASK{Q4B};


class ArithmeticCoder {
    std::size_t m_L{0};
    std::size_t m_R{Q4B};
    std::size_t m_waiting{0}; // Re-use for encoding and decoding
    BitVec &m_outbits;
    BitVec::const_iterator m_iter;
public:
    struct Probs {
        double excmf;
        double pmf;
    };
    struct ScaledProbs {
        std::size_t excmf;
        std::size_t pmf;
    };
private:

    std::size_t scale_prob(double prob) const {
        return static_cast<std::size_t>(prob * static_cast<double>(m_R));
    }
    double inv_scale_prob(std::size_t target) const {
        return static_cast<double>(target)/static_cast<double>(m_R);
    }
    void narrow_region(ScaledProbs sprobs) {
        m_L += sprobs.excmf;
        m_R = sprobs.pmf;
    }
    void out_all(bit_t bit) {
        m_outbits.push_back(bit);
        m_outbits.insert(m_outbits.end(), m_waiting, flipbit(bit));
        m_waiting = 0;
    }
    void output_bits() {
        while (m_R <= Q1B) {
            if (m_R + m_L <= Q2B) {
                out_all(bit_t::ZERO);
            } else if (m_L >= Q2B) {
                out_all(bit_t::ONE);
                m_L -= Q2B;
            } else {
                m_waiting += 1;
                m_L -= Q1B;
            }
            m_L <<= 1;
            m_R <<= 1;
        }
    }
    void read_bit_in() {
        // Read in zero instead of panicking
        auto bit = m_iter == m_outbits.cend() ? bit_t::ZERO : *m_iter++;
        m_waiting <<= 1;
        m_waiting &= MASK;
        m_waiting += to_underlying(bit);
    }
    void discard_bits() {
        while (m_R <= Q1B) {
            if (m_L >= Q2B) {
                m_L -= Q2B;
                m_waiting -= Q2B;
            } else if (m_L + m_R <= Q2B) {
                // Lower half, do nothing
            } else {
                m_L -= Q1B;
                m_waiting -= Q1B;
            }
            m_L <<= 1;
            m_R <<= 1;
            read_bit_in();
        }
    }
public:
    ArithmeticCoder (BitVec &compressed) : m_outbits{compressed},
                                           m_iter{m_outbits.cbegin()} {
        // Compressor Mode
        if (compressed.empty()) {
            return;
        }
        // Decompressor Mode
        for (auto i = 0UL ; i < WHOLEB; ++i) {
            read_bit_in();
        }
    }
    void encode(Probs const &probs) {
        ScaledProbs sprobs = {
            .excmf=scale_prob(probs.excmf),
            .pmf=scale_prob(probs.pmf)
        };
        assert(sprobs.pmf > 0);
        narrow_region(sprobs);
        output_bits();
    }
    void finish_encode() {
        for(;;) {
            if (m_L + (m_R >> 1) >= Q2B) {
                out_all(bit_t::ONE);
                if (m_L < Q2B) {
                    m_R -= Q2B - m_L;
                    m_L =0;
                } else {
                    m_L -= Q2B;
                }
            } else {
                out_all(bit_t::ZERO);
                if (m_L+m_R > Q2B) {
                    m_R = Q2B - m_L;
                }
            }
            if (m_R == Q2B) {
                break;
            }
            m_L <<= 1;
            m_R <<= 1;
        }
    }
    BitVec get_compressed() const {
        return m_outbits;
    }
    BitVec tear_out() {
        return std::move(m_outbits);
    }
    template <typename ModelT>
    typename ModelT::Alphabet::sym_t decode(ModelT const &model) {
        auto target = m_waiting - m_L;
        auto ret = find_sym_from_cum_prob(model, inv_scale_prob(target));
        narrow_region(ScaledProbs{.excmf=scale_prob(excmf(model, ret)),
                                  .pmf=scale_prob(pmf(model, ret))});
        discard_bits();
        return ret;
    }
};

template <typename ModelT>
class StreamingACEnc {
    ModelT m_model;
    BitVec &m_bv;
    ArithmeticCoder m_ac;
public:
    using sym_t = typename ModelT::Alphabet::sym_t;
    StreamingACEnc(BitVec &bv, ModelT&& model)
        : m_model{std::forward<ModelT>(model)}
        , m_bv{bv}
        , m_ac{bv} {}
    void encode(sym_t const &sym) {
        ArithmeticCoder::Probs probs{
            .excmf = excmf(m_model, sym),
            .pmf = pmf(m_model, sym),
        };
        m_ac.encode(probs);
        m_model.learn(sym);
    }
    ~StreamingACEnc() {
        m_ac.finish_encode();
    }
};
template <typename ModelT>
class StreamingACDec {
    ModelT m_model;
    BitVec m_bv;
    ArithmeticCoder m_ac;
public:
    using sym_t = typename ModelT::Alphabet::sym_t;
    StreamingACDec(BitVec &&bv, ModelT &&model)
        : m_model{std::forward<ModelT>(model)}
        , m_bv{std::move(bv)}
        , m_ac{m_bv} {}
    sym_t decode() {
        auto sym = m_ac.decode(m_model);
        m_model.learn(sym);
        return sym;
    }
};
