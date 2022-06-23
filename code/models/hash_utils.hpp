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
