#pragma once
#include "model_sequence.hpp"

#include "volfctw.hpp"
#include "sequencememoizer.hpp"
#include "hashing.hpp"
#include "hash_methods.hpp"
#include "hashing_hamming.hpp"
// template <typename AlphabetT>
// using VolfCTWModel = SequenceModel<AmortizedVolf<AlphabetT::size>,
//                                    AlphabetT>;
// template <typename AlphabetT>
// using AmnesiaVolfCTWModel = AmnesiaSequenceModel<AmortizedVolf<AlphabetT::size>,
//                                                  AlphabetT>;
// template <typename AlphabetT>
// using HashCTWModel = SequenceModel<HashVolf<AlphabetT::size>,
//                                    AlphabetT>;


template <typename AlphabetT>
using SMUKNModel = SequenceModel<AmortizedSM<SMUKNHistogram<AlphabetT::size>>,
                                 AlphabetT>;
template <typename AlphabetT>
using HashSMUKNModel = SequenceModel<HasherTopDown<RandomLookup,
                                                   SMUKNHistogram<AlphabetT::size>>,
                                     AlphabetT>;

template <typename AlphabetT>
using AmnesiaSMUKNModel = AmnesiaSequenceModel<AmortizedSM<SMUKNHistogram<AlphabetT::size>>,
                                               AlphabetT>;
template <typename AlphabetT>
using HashPureZCTXSMUKNModel = SequenceModel<HasherTopDown<PureZCTXLookup,
                                                           SMUKNHistogram<AlphabetT::size>>,
                                             AlphabetT>;



template <typename AlphabetT>
using SM1PFModel = SequenceModel<AmortizedSM<SM1PFHistogram<AlphabetT::size>>,
                                 AlphabetT>;
template <typename AlphabetT>
using HashSM1PFModel = SequenceModel<HasherTopDown<RandomLookup, SM1PFHistogram<AlphabetT::size>>,
                                     AlphabetT>;
template <typename AlphabetT>
using AmnesiaSM1PFModel = AmnesiaSequenceModel<AmortizedSM<SM1PFHistogram<AlphabetT::size>>,
                                               AlphabetT>;
template <typename AlphabetT>
using HashPureZCTXSM1PFModel = SequenceModel<HasherTopDown<PureZCTXLookup,
                                                           SM1PFHistogram<AlphabetT::size>>,
                                             AlphabetT>;
