#pragma once
#include "model_sequence.hpp"
#include "volfctw.hpp"
#include "ppmdp.hpp"
#include "sequencememoizer.hpp"
#include "hashing.hpp"
#include "hash_methods.hpp"

// CTW
template <typename AlphabetT>
using VolfCTWModel = SequenceModel<
    AmortizedVolf<
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using AmnesiaVolfCTWModel= AmnesiaSequenceModel<
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
using LengthBucketHashCTWModel = SequenceModel<
    HasherBottomUp<
        LengthBucketLookup,
        VolfHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using DepthSeededHashCTWModel = SequenceModel<
    HasherBottomUp<
        DepthSeededLookup,
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
using AmnesiaSMUKNModel = AmnesiaSequenceModel<
    AmortizedSM<
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using HashPureZCTXSMUKNModel = SequenceModel<
    HasherTopDown<
        PureZCTXLookup,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using LengthBucketHashSMUKNModel = SequenceModel<
    HasherTopDown<
        LengthBucketLookup,
        SMUKNHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using DepthSeededHashSMUKNModel = SequenceModel<
    HasherTopDown<
        DepthSeededLookup,
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
using AmnesiaSM1PFModel = AmnesiaSequenceModel<
    AmortizedSM<
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using HashPureZCTXSM1PFModel = SequenceModel<
    HasherTopDown<
        PureZCTXLookup,
        SM1PFHistogram<
            AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using LengthBucketHashSM1PFModel = SequenceModel<
    HasherTopDown<
        LengthBucketLookup,
        SM1PFHistogram<AlphabetT::size>>,
    AlphabetT>;
template <typename AlphabetT>
using DepthSeededHashSM1PFModel = SequenceModel<
    HasherTopDown<
        DepthSeededLookup,
        SM1PFHistogram<AlphabetT::size>>,
    AlphabetT>;



template <typename AlphabetT>
using PPMDPModel = SequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using PPMDPFullModel = SequenceModel<
    ExactProbDownLearnUp<
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::FullUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using HashPPMDPModel = SequenceModel<
    HasherProbDownLearnUp<
        RandomLookup,
        PPMDPHistogram<AlphabetT::size>,
        PPMUpdatePolicy::ShallowUpdates>,
    AlphabetT>;
template <typename AlphabetT>
using HashPPMDPFullModel = SequenceModel<
    HasherProbDownLearnUp<
        RandomLookup,
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
// template <typename AlphabetT>
// using HashPureZCTXNBSMUKNModel = SequenceModel<
//     HasherTopDown<
//         PureZCTXLookup,
//         NBSMUKNHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using LengthBucketHashNBSMUKNModel = SequenceModel<
//     HasherTopDown<
//         LengthBucketLookup,
//         NBSMUKNHistogram<AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using DepthSeededHashNBSMUKNModel = SequenceModel<
//     HasherTopDown<
//         DepthSeededLookup,
//         NBSMUKNHistogram<AlphabetT::size>>,
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
// template <typename AlphabetT>
// using HashPureZCTXNBSM1PFModel = SequenceModel<
//     HasherTopDown<
//         PureZCTXLookup,
//         NBSM1PFHistogram<
//             AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using LengthBucketHashNBSM1PFModel = SequenceModel<
//     HasherTopDown<
//         LengthBucketLookup,
//         NBSM1PFHistogram<AlphabetT::size>>,
//     AlphabetT>;
// template <typename AlphabetT>
// using DepthSeededHashNBSM1PFModel = SequenceModel<
//     HasherTopDown<
//         DepthSeededLookup,
//         NBSM1PFHistogram<AlphabetT::size>>,
//     AlphabetT>;
