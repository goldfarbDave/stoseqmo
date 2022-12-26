#pragma once
// From xa8_d7f12 under https://github.com/q4/ppm-dp/blob/master/java/PPMDP.java
#include <array>
#include <algorithm>
#include "data_types.hpp"
using hpmat_t = std::array<std::array<prob_t, 12>, 7>;
constexpr hpmat_t g_alphas{{
    {{2.670471754898557,0.4069156650423885,1.0274940338735652,3.8174973823955947,3.0174437303500254,2.4937152122216517,2.1391928826956983,1.8895186387960137,1.706978970353831,0.8364172686087561,1.5496238741210386,14.68738468564935}},
    {{-0.2758467323138136,-0.1529959748382007,0.11943674087760114,0.11776879195944867,-0.3597364448078298,3.0906017270463155,3.4637556617510867,6.604316106499125,6.201898658383731,12.968574421594363,6.664480201284984,5.718403497485102}},
    {{0.4759444694095466,0.9133674107978305,1.645037791693202,1.5116559769278144,1.8039891946796442,0.7222392992861796,0.8795771934068553,1.3240631565616576,8.979741729506424,4.355607563619354,8.361304746135323,8.540283208373767}},
    {{0.7076224427008381,1.3458562494407575,1.827226057855866,1.0954926349627725,3.3892036855759597,2.476186642413814,3.325404504257986,3.0768289077511684,3.932974131110006,4.504463364625853,2.621768311798954,9.130315576897159}},
    {{0.41202006414310577,1.0777018626960388,2.091413689864072,2.4119842295028424,4.767722079904576,3.6248251187431575,4.247495297355498,6.34447996781849,14.90656628410044,20.230425910907822,13.73650527123939,17.20229102399932}},
    {{0.4468505714047082,1.110564115869042,1.4224043747460093,2.7512237510581254,2.8698540225764173,4.221550588495045,3.9674813256442305,5.608947923418591,13.555432679570387,7.778368657544978,16.681524873696006,26.748167292055253}},
    {{0.5782926932538701,1.2339156504962188,2.5152568924694063,5.238639265996976,5.804457460368215,6.71726106464864,9.611084390063628,8.598855415782584,11.493486660560725,14.543650628199964,8.985008652654955,30.135557002770778}}}};
constexpr inline hpmat_t g_betas{{
    {{0.6028449481079561,0.2705280037968433,0.3221969894012631,0.7113157540770672,0.7352031421248162,0.7513801325682694,0.7627289505617817,0.7709869976594529,0.7771995304101451,0.6387327220782311,0.7848084681042061,0.0033047082693115704}},
    {{0.8410035829870186,0.7679571770559815,0.7202185373846259,0.7026912922879597,0.6086406299616743,0.4504162888007199,0.49932503626076585,0.3771332207922993,0.3644433471697172,0.24900795211206836,0.24564293677276247,0.3748430145584661}},
    {{0.6524053708124778,0.6501361255764491,0.5065443573762902,0.6327283546456011,0.5090743607343395,0.6861010994797739,0.6369604190117211,0.6620681874992546,0.47145568285315453,0.6958529326334474,0.5828635663374603,0.7156879493325492}},
    {{0.5059487857527313,0.5757229941902429,0.6422965842741565,0.7400261271627675,0.6073299519294538,0.6055404724935816,0.6418909099772226,0.6685108646000572,0.7672394576220453,0.6894913112801403,0.732075225223134,0.8872605005257956}},
    {{0.6929614620257987,0.7332534200528583,0.8703114130665186,0.9038076180200592,0.8605380081144592,0.9374529532707708,0.8972365362986254,0.8565595508692613,0.7971215497646632,0.9653082068064397,0.988023234993729,0.9520487514480321}},
    {{0.7826474240460997,0.893659746309123,0.8831069982622155,0.936298942414761,0.9566272309067896,0.9273899432706725,0.9401322850323148,0.9850389791512117,0.900899453525255,0.9131520829448182,0.9923444711755682,0.8076007191861467}},
    {{0.8351095859103213,0.8971167952583056,0.9184012017579287,0.8960120419968413,0.8902395707253267,0.8450680251858269,0.8189732069484699,0.9670448184950308,0.9039173589002338,0.9074502081657243,0.8394470430823417,0.8903725427145421}}}};


struct BlendParamsIn{
    std::size_t depth{};
    std::size_t fanout{};
};
struct BlendParamsOut{
    prob_t alpha{};
    prob_t beta{};
};
BlendParamsOut get_blend_params(BlendParamsIn const& inp) {
    auto depth = std::min(inp.depth, 6UL);
    auto fanout = std::min(std::max(inp.fanout, 1UL)-1, 11UL);

    return BlendParamsOut{
        .alpha=g_alphas[depth][fanout],
        .beta=g_betas[depth][fanout]
    };
}