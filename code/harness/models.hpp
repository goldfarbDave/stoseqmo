#pragma once
#include "model_sequence.hpp"

#include "volfctw.hpp"
#include "sequencememoizer.hpp"
#include "hashing.hpp"


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
using SMUKNModel = SequenceModel<AmortizedSM<SMUKNHistogram<AlphabetT::size>>,
                                            AlphabetT>;
template <typename AlphabetT>
using HashSMUKNModel = SequenceModel<HashTopDown<SMUKNHistogram<AlphabetT::size>>,
                                                AlphabetT>;
template <typename AlphabetT>
using SM1PFModel = SequenceModel<AmortizedSM<SM1PFHistogram<AlphabetT::size>>,
                                            AlphabetT>;
template <typename AlphabetT>
using HashSM1PFModel = SequenceModel<HashTopDown<SM1PFHistogram<AlphabetT::size>>,
                                                AlphabetT>;
