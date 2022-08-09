#include "eeds/countset.h"

using Set = eeds::countset<int>;
static_assert(std::is_same_v<Set::key_type, int>);
static_assert(std::is_same_v<Set::value_type, int>);
static_assert(std::is_same_v<Set::size_type, std::size_t>);
static_assert(std::is_same_v<Set::difference_type, std::ptrdiff_t>);
static_assert(std::is_same_v<Set::key_compare, std::less<>>);
static_assert(std::is_same_v<Set::value_compare, std::less<>>);
static_assert(std::is_same_v<Set::allocator_type, std::allocator<int>>);
static_assert(std::is_same_v<Set::reference, int&>);
static_assert(std::is_same_v<Set::const_reference, const int&>);
static_assert(std::is_same_v<Set::pointer, int*>);
static_assert(std::is_same_v<Set::const_pointer, const int*>);
static_assert(std::is_same_v<Set::node_type, eeds::countset_node<int>>);
static_assert(std::is_same_v<Set::rng_type, eeds::default_rng>);
static_assert(
    std::is_same_v<Set::iterator,
                   eeds::countset_iterator<int, eeds::iterator_dir::fwd>>);
static_assert(
    std::is_same_v<Set::const_iterator,
                   eeds::countset_iterator<int, eeds::iterator_dir::fwd>>);
static_assert(
    std::is_same_v<Set::reverse_iterator,
                   eeds::countset_iterator<int, eeds::iterator_dir::rev>>);
static_assert(
    std::is_same_v<Set::const_reverse_iterator,
                   eeds::countset_iterator<int, eeds::iterator_dir::rev>>);
static_assert(std::is_same_v<decltype(std::declval<Set&>().get_allocator()),
                             Set::allocator_type>);
static_assert(std::is_same_v<decltype(std::declval<Set&>().key_comp()),
                             Set::key_compare>);
static_assert(std::is_same_v<decltype(std::declval<Set&>().value_comp()),
                             Set::value_compare>);
static_assert(
    std::is_same_v<decltype(std::declval<Set&>().get_rng()), Set::rng_type>);
static_assert(Set::max_size() == std::numeric_limits<std::size_t>::max());

// Ensure all functions are instantiated, so we have accurate coverage
// information.
template struct eeds::countset<int>;
