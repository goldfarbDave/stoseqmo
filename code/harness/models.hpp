#pragma once
#include "model_sequence.hpp"

#include "volfctw.hpp"
#include "sequencememoizer.hpp"
#include "hashing.hpp"
template <typename AlphabetT>
using SequenceMemoizerModel = SequenceModel<AmortizedSM<AlphabetT::size>,
                                            AlphabetT>;
template <typename AlphabetT>
using VolfCTWModel = SequenceModel<AmortizedVolf<AlphabetT::size>,
                                   AlphabetT>;

template <typename AlphabetT>
using AmnesiaVolfCTWModel = AmnesiaSequenceModel<AmortizedVolf<AlphabetT::size>,
                                                 AlphabetT>;

template <typename AlphabetT>
using HashCTWModel = SequenceModel<HashVolf<AlphabetT::size>,
                                   AlphabetT>;
template <typename AlphabetT>
using HashSequenceMemoizerModel = SequenceModel<HashSequenceMemoizer<AlphabetT::size>,
                                                AlphabetT>;
