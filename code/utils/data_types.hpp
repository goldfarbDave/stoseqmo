#pragma once
// Utilities to deal with strongly-typed bits and bytes
#include <array>
#include <cassert>
#include <type_traits>
#include <vector>

// A sprinkle of strong typing
template <typename E>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
enum class bit_t : uint8_t {ZERO=0, ONE=1};
enum class byte_t : uint8_t {};
using idx_t = std::size_t;

using BitVec = std::vector<bit_t>;
bit_t flipbit(bit_t bit) {
    return bit == bit_t::ONE ? bit_t::ZERO : bit_t::ONE;
}

// namespace std {
//     template <>
//     struct hash<bit_t> {
//         size_t operator()(const bit_t& bit) const {
//             return std::hash<std::underlying_type_t<bit_t>>()(to_underlying(bit));
//         }
//     };
// }


// Compile-time description of alphabet
struct ByteAlphabet {
    static constexpr std::size_t size = 256;
    using sym_t=byte_t;
    static constexpr std::underlying_type_t<sym_t> to_idx(sym_t el) {
        return to_underlying(el);
    }
    template <typename T>
    static constexpr sym_t to_sym(T idx) {
        assert(idx < static_cast<int>(size));
        return sym_t(idx);
    }
};
struct BitAlphabet {
    static constexpr std::size_t size = 2;
    using sym_t =bit_t;
    static constexpr std::underlying_type_t<sym_t> to_idx(sym_t el) {
        return to_underlying(el);
    }
    template <typename T>
    static constexpr sym_t to_sym(T idx) {
        assert(idx < size);
        return sym_t(idx);
    }
};

std::array<bit_t, 8> byte_to_bits(byte_t byte) {
    //const auto byte_digits = std::numeric_limits<std::underlying_type_t<byte_t>>::digits;
    constexpr auto byte_digits = 8;
    std::array<bit_t, byte_digits> ret;
    for (auto i = 0; i < byte_digits; ++i) {
        ret[i] = bit_t((to_underlying(byte) >> (byte_digits-i-1)) & 1);
    }
    return ret;
}
std::vector<bit_t> bytevec_to_bitvec(std::vector<byte_t> const &bytevec) {
    std::vector<bit_t> ret;
    for (auto const &byte : bytevec) {
        auto bitvec = byte_to_bits(byte);
        ret.insert(ret.end(), bitvec.begin(), bitvec.end());
    }
    return ret;
}
