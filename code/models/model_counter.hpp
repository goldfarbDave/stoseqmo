#pragma once
#include <array>
#include <numeric>
#include "data_types.hpp"
template <typename CountT, std::size_t N>
class RescalingCounter {
    std::array<CountT, N> m_data{};
    std::size_t m_tot{};
    constexpr static CountT RESCALE_THRESHOLD = std::numeric_limits<CountT>::max();
public:
    void increment(idx_t sym) {
        m_data[sym] += 1;
        m_tot += 1;
        if (m_data[sym] == RESCALE_THRESHOLD) {
            auto new_total = 0;
            std::transform(m_data.begin(), m_data.end(), m_data.begin(),
                           [this, &new_total](auto const &el) {
                               // Divide by 2, round up
                               auto nc =(el >> 1) + (el & 1);
                               new_total += nc;
                               return nc;
                           });
            m_tot = new_total;
        }
    }
    std::size_t total() const {
        return m_tot;
    }
    CountT operator[](idx_t sym) const {
        return m_data[sym];
    }
    auto begin() const {
        return m_data.begin();
    }
    auto end() const {
        return m_data.end();
    }
};
