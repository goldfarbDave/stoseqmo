#pragma once
#include "model_sequence.hpp"

#include "volfctw.hpp"
#include "sequencememoizer.hpp"

template <typename AlphabetT>
using SequenceMemoizerModel = SequenceModel<AmortizedSM<AlphabetT>>;

template <typename AlphabetT>
using VolfCTWModel = SequenceModel<AmortizedVolf<AlphabetT>>;

template <typename AlphabetT>
using AmnesiaVolfCTWModel = AmnesiaSequenceModel<AmortizedVolf<AlphabetT>>;