// Utilities for using models (models should not include this file)
#pragma once
#include <algorithm>
#include <numeric>
#include <iterator>
#include <cmath>
#include <utility>
template <typename ModelT>
double pmf(ModelT const& model, typename ModelT::Alphabet::sym_t sym) {
    auto idx = ModelT::Alphabet::to_idx(sym);
    return model.get_probs()[idx];
}
template <typename ModelT>
double excmf(ModelT const& model, typename ModelT::Alphabet::sym_t sym) {
    auto probar = model.get_probs();
    auto cmf = std::accumulate(probar.begin(),
                               probar.begin()+ModelT::Alphabet::to_idx(sym),
                               0.0);
    return cmf;
}
template <typename ModelT>
typename ModelT::Alphabet::sym_t find_sym_from_cum_prob(ModelT const& model, double cum_prob) {
    auto probar = model.get_probs();
    auto pos = std::find_if(probar.begin(), probar.end(),
                            [&cum_prob](auto const& prob) {
                                if (cum_prob < prob) {
                                    return true;
                                }
                                cum_prob -= prob;
                                return false;
                            });
    return ModelT::Alphabet::to_sym(std::distance(probar.begin(), pos));
}

template <typename ModelT>
struct EntropyRun {
    ModelT model;
    double H;
};

template <typename ModelT>
EntropyRun<ModelT> entropy_of_model(std::vector<typename ModelT::Alphabet::sym_t> const &syms, ModelT &&model) {
    double H = 0.0;
    for (auto const & sym: syms) {
        H += -log2(pmf(model, sym));
        model.learn(sym);
    }
    //std::cout << make_size_string(model);
    return EntropyRun<ModelT>{.model=std::move(model), .H=H};
}
