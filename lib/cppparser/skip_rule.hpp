#ifndef SKIP_RULE_H
#define SKIP_RULE_H

#include <boost/spirit/include/classic.hpp>

/** This header should (hopefully) be part of a future release of
    boost::spirit */

namespace boost {
namespace spirit {
namespace classic {
namespace ext {

template < typename RuleT, typename BaseT = iteration_policy >
class skip_rule_iteration_policy
            : public skipper_iteration_policy<BaseT>
{
public:

    typedef skipper_iteration_policy<BaseT> base_t;
    typedef typename RuleT::scanner_t skip_rule_scanner_t;
    typedef typename skip_rule_scanner_t::policies_t skip_rule_policies_t;

    skip_rule_iteration_policy(RuleT const& skip_rule,
                               skip_rule_policies_t const& policies
                               = skip_rule_policies_t(),
                               base_t const& base = base_t())
            : base_t(base), subject(skip_rule), _policies(policies) {}

    template <typename PolicyT>
    skip_rule_iteration_policy(PolicyT const& other)
            : base_t(other), subject(other.skipper()),
            _policies(other.policies()) {}

    template <typename ScannerT>
    void skip(ScannerT const& scan) const {
        skip_rule_scanner_t skip_scanner(scan.first, scan.last, _policies);
        impl::skipper_skip(subject, skip_scanner, skip_scanner);
    }

    RuleT const& skipper() const {
        return subject;
    }

    skip_rule_policies_t const& policies() const {
        return _policies;
    }

private:
    RuleT const& subject;
    skip_rule_policies_t const& _policies;
};

template < typename SkipRuleT
, typename IteratorT = char const*
, typename SkipMatchPolicyT = match_policy
, typename SkipActionPolicyT = action_policy >
struct skip_rule_parser {
    typedef IteratorT iterator_t;
    typedef SkipRuleT skip_rule_t;
    typedef
    typename skip_rule_t::scanner_t::policies_t skip_rule_policies_t;
    typedef skip_rule_iteration_policy<skip_rule_t> skip_iteration_policy_t;
    typedef SkipMatchPolicyT skip_match_policy_t;
    typedef SkipActionPolicyT skip_action_policy_t;
    typedef scanner_policies <
    skip_iteration_policy_t
    , skip_match_policy_t
    , skip_action_policy_t >
    skip_scanner_policies_t;
    typedef scanner<iterator_t, skip_scanner_policies_t>
    skip_scanner_t;

    template<typename ParserT>
    static parse_info<iterator_t>
    parse(iterator_t const& first_,
          iterator_t const& last,
          parser<ParserT> const& p,
          skip_rule_t const& skip,
          skip_match_policy_t const& smp = match_policy(),
          skip_action_policy_t const& sap = action_policy(),
          skip_rule_policies_t const& srps = skip_rule_policies_t()) {
        skip_iteration_policy_t sip(skip, srps);
        skip_scanner_policies_t policies(sip, smp, sap);
        iterator_t first = first_;
        skip_scanner_t scan(first, last, policies);
        match<nil_t> hit = p.derived().parse(scan);
        scan.skip(scan);
        return parse_info<iterator_t>
               (first, hit, hit && (first == last), hit.length());
    }
};

}  // end namespace ext
}  // end namespace classic
}  // end namespace spirit
}  // end namespace boost

#endif
