#pragma once
template <typename BackingSchemeT>
class TopDownTraversal {
private:
    using token_t = typename BackingSchemeT::token_t;
    using Node = typename BackingSchemeT::Node;
    std::size_t m_depth;
    BackingSchemeT m_backing;
    using ProbAr = decltype(Node::get_prior());
    Node& lu(token_t const &token) {
        return m_backing[token];
    }
    Node const & lu(token_t const &token) const {
        return m_backing[token];
    }
    struct PathProb {
        token_t token;
        ProbAr parent_prob;
    };
    std::vector<PathProb> ctx_to_path_prob(IdxContext ctx) {
        std::vector<PathProb> ret;
        auto tokens = m_backing.make_ctx_to_tokens(ctx);
        ret.reserve(m_depth+1);
        ret.push_back(PathProb{.token=tokens[0], .parent_prob=Node::get_prior()});
        auto depth = 0;
        for (auto token_itr = std::next(tokens.cbegin()); token_itr != tokens.cend(); ++token_itr) {
            ret.push_back(
                PathProb{.token=*token_itr,
                         .parent_prob=lu(*token_itr).transform_probs(ret.back().parent_prob, depth++)
                });
        }
        return ret;
    }
public:
    template <typename... Args>
    TopDownTraversal (std::size_t depth, Args &&... args)
        : m_depth{depth}
        , m_backing(depth, std::forward<Args>(args)...)  {}

    void learn(IdxContext const &ctx, idx_t const & sym) {
        // Naive approach, build up counts, then iterate backwards (deeper to shallower context)
        auto token_pps = ctx_to_path_prob(ctx);
        auto depth{token_pps.size()};
        for (auto itr = token_pps.rbegin(); itr != token_pps.rend(); ++itr) {
            if (lu(itr->token).update_counts(sym, itr->parent_prob, depth--)) {
                break;
            }
        }
    }
    ProbAr get_probs(IdxContext ctx) const {
        auto tokens = m_backing.ctx_to_tokens(ctx);
        auto depth = 0;
        auto const prior = Node::get_prior();
        auto ret = std::accumulate(std::next(tokens.cbegin()), tokens.cend(),
                                   lu(tokens[0]).transform_probs(prior, depth++),
                                   [&depth, this](auto const &prob_ar, auto const &token) {
                                       return lu(token).transform_probs(prob_ar, depth++);
                                   });
        // Mix:
        std::transform(ret.begin(), ret.end(), prior.begin(), ret.begin(),
                       [](auto const &r, auto const &p) {
                           return (99*r + p)/100;
                       });
        return ret;
    }
    Footprint footprint() const {
        return m_backing.footprint();
    }
};
