#pragma once
#include "model_sequence.hpp"
#include "volfctw.hpp"
#include "ppmdp.hpp"
#include "sequencememoizer.hpp"
#include "hashing.hpp"
#include "hash_methods.hpp"

#define LIST_OF_MODELS \
    X(CTW) \
    X(SM1PF) \
    X(SMUKN) \
    X(PPMDP) \
    X(PPMDPFull)


// CTW
template <typename AlphabetT>
using CTWModel = SequenceModel<
    AmortizedVolf<
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaCTWModel= AmnesiaSequenceModel<
    AmortizedVolf<
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using HashCTWModel = SequenceModel<
    HasherBottomUp<
        RandomLookup,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using FNVHashCTWModel = SequenceModel<
    HasherBottomUp<
        FNVLookup,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure0HashCTWModel = SequenceModel<
    HasherBottomUp<
        PureLowCtxLookup<1>,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure1HashCTWModel = SequenceModel<
    HasherBottomUp<
        PureLowCtxLookup<2>,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure2HashCTWModel = SequenceModel<
    HasherBottomUp<
        PureLowCtxLookup<3>,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;


// UKN
template <typename AlphabetT>
using SMUKNModel = SequenceModel<
    AmortizedSM<
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using HashSMUKNModel = SequenceModel<
    HasherTopDown<
        RandomLookup,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using FNVHashSMUKNModel = SequenceModel<
    HasherTopDown<
        FNVLookup,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaSMUKNModel = AmnesiaSequenceModel<
    AmortizedSM<
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;

template <typename AlphabetT>
using Pure0HashSMUKNModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<1>,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure1HashSMUKNModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<2>,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure2HashSMUKNModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<3>,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;

// 1PF
template <typename AlphabetT>
using SM1PFModel = SequenceModel<
    AmortizedSM<
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using HashSM1PFModel = SequenceModel<
    HasherTopDown<
        RandomLookup,
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using FNVHashSM1PFModel = SequenceModel<
    HasherTopDown<
        FNVLookup,
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaSM1PFModel = AmnesiaSequenceModel<
    AmortizedSM<
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure0HashSM1PFModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<1>,
        SM1PFHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure1HashSM1PFModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<2>,
        SM1PFHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using Pure2HashSM1PFModel = SequenceModel<
    HasherTopDown<
        PureLowCtxLookup<3>,
        SM1PFHistogram<AlphabetT::size>>,
    AlphabetT>;


// PPM-DP Shallow
template <typename AlphabetT>
using PPMDPModel = SequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using HashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        RandomLookup,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using FNVHashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        FNVLookup,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaPPMDPModel = AmnesiaSequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure0HashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<1>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure1HashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<2>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure2HashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<3>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
// PPM-DP Full
template <typename AlphabetT>
using PPMDPFullModel = SequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using HashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        RandomLookup,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using FNVHashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        FNVLookup,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaPPMDPFullModel = AmnesiaSequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure0HashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<1>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure1HashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<2>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using Pure2HashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        PureLowCtxLookup<3>,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;

// NB SM variants
// NBUKN
// template <std::size_t size>
// using NBSMUKNHistogram = SMUKNHistogram<size, false>;
// template <std::size_t size>
// using NBSM1PFHistogram = SM1PFHistogram<size, false>;
// template <typename AlphabetT>
// using NBSMUKNModel = SequenceModel<
//     AmortizedSM<
//         NBSMUKNHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using HashNBSMUKNModel = SequenceModel<
//     HasherTopDown<
//         RandomLookup,
//         NBSMUKNHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using AmnesiaNBSMUKNModel = AmnesiaSequenceModel<
//     AmortizedSM<
//         NBSMUKNHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// // NB1PF
// template <typename AlphabetT>
// using NBSM1PFModel = SequenceModel<
//     AmortizedSM<
//         NBSM1PFHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using HashNBSM1PFModel = SequenceModel<
//     HasherTopDown<
//         RandomLookup,
//         NBSM1PFHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using AmnesiaNBSM1PFModel = AmnesiaSequenceModel<
//     AmortizedSM<
//         NBSM1PFHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
