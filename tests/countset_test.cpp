#include "countset_test.h"

#include "eeds/countset.h"

#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <vector>

using eeds::countset;
using eeds::countset_iterator;
using eeds::countset_node;
using eeds::default_rng;
using eeds::iterator_dir;

// Ensure all functions are instantiated, so we have accurate coverage
// information.
template struct eeds::countset<int>;

struct counted_ops {
  static inline int default_ctor_count = 0;
  static inline int value_ctor_count = 0;
  static inline int copy_ctor_count = 0;
  static inline int move_ctor_count = 0;
  static inline int copy_assign_count = 0;
  static inline int move_assign_count = 0;
  static inline int dtor_count = 0;
  static void clear_counters() {
    default_ctor_count = 0;
    value_ctor_count = 0;
    copy_ctor_count = 0;
    move_ctor_count = 0;
    copy_assign_count = 0;
    move_assign_count = 0;
    dtor_count = 0;
  }

  int value;

  counted_ops(): value{0} {
    default_ctor_count++;
  }
  explicit counted_ops(int val): value{val} {
    value_ctor_count++;
  }
  counted_ops(const counted_ops& other): value{other.value} {
    copy_ctor_count++;
  }
  counted_ops(counted_ops&& other) noexcept: value{other.value} {
    move_ctor_count++;
  }
  counted_ops& operator=(const counted_ops& other) {
    value = other.value;
    copy_assign_count++;
    return *this;
  }
  counted_ops& operator=(counted_ops&& other) noexcept {
    value = other.value;
    move_assign_count++;
    return *this;
  }
  ~counted_ops() {
    dtor_count++;
  }

  [[nodiscard]] auto operator<=>(const counted_ops& other) const noexcept
      = default;
};

struct wrapped_ints {
  int value;
  int second_value;

  [[nodiscard]] auto operator<=>(const wrapped_ints& other) const noexcept
      = default;

  // A transparent comparison of just the first integer of the pair.
  [[nodiscard]] auto operator<=>(const int& other) const noexcept {
    return value <=> other;
  }
};

TEST(countset, types) {
  using Set = countset<int>;
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
  static_assert(std::is_same_v<Set::node_type, countset_node<int>>);
  static_assert(std::is_same_v<Set::rng_type, default_rng>);
  static_assert(
      std::is_same_v<Set::iterator, countset_iterator<int, iterator_dir::fwd>>);
  static_assert(std::is_same_v<Set::const_iterator,
                               countset_iterator<int, iterator_dir::fwd>>);
  static_assert(std::is_same_v<Set::reverse_iterator,
                               countset_iterator<int, iterator_dir::rev>>);
  static_assert(std::is_same_v<Set::const_reverse_iterator,
                               countset_iterator<int, iterator_dir::rev>>);
}

TEST(countset, default_ctor) {
  countset<int> c;
  assert_countset_is(c, {});

  // Make sure it's implicit
  using T = int;
  using C = std::less<>;
  using A = std::allocator<int>;
  using R = default_rng;
  assert_countset_is<T, C, A, R>({}, {});
}

TEST(countset, init_list_ctor) {
  countset<int> c{1, 3, 5};
  assert_countset_is(c, {1, 3, 5});

  // Make sure it's implicit
  using T = int;
  using C = std::less<>;
  using A = std::allocator<int>;
  using R = default_rng;
  assert_countset_is<T, C, A, R>({1, 3, 5}, {1, 3, 5});
}

TEST(countset, iterator_pair_ctor) {
  {
    // random access
    std::vector<int> v{1, 3, 5};
    countset<int> c(v.begin(), v.end());
    assert_countset_is(c, {1, 3, 5});
  }
  {
    // bidirectional
    std::list<int> v{1, 3, 5};
    countset<int> c(v.begin(), v.end());
    assert_countset_is(c, {1, 3, 5});
  }
  {
    // input
    std::stringstream sin{"1 3 5"};
    countset<int> c(std::istream_iterator<int>{sin},
                    std::istream_iterator<int>{});
    assert_countset_is(c, {1, 3, 5});
  }
}

TEST(countset, copy_ctor) {
  auto* c = new countset<int>{1, 3, 5};
  countset<int> c2(*c);  // NOLINT(performance-unnecessary-copy-initialization)
  delete c;
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, copy_ctor_empty) {
  auto* c = new countset<int>{};
  countset<int> c2(*c);  // NOLINT(performance-unnecessary-copy-initialization)
  delete c;
  assert_countset_is(c2, {});
}

TEST(countset, move_ctor) {
  auto* c = new countset<int>{1, 3, 5};
  countset<int> c2(std::move(*c));
  delete c;
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, move_ctor_empty) {
  auto* c = new countset<int>{};
  countset<int> c2(std::move(*c));
  delete c;
  assert_countset_is(c2, {});
}

TEST(countset, copy_assign) {
  countset<int> c2;
  auto* c = new countset<int>{1, 3, 5};
  c2 = *c;
  delete c;
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-assignment works
  // Wrap in a lambda to silence a compiler warning about this.
  constexpr auto self_assign = [](auto& x, auto& y) {
    x = y;
  };
  self_assign(c2, c2);
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, move_assign) {
  countset<int> c2;
  auto* c = new countset<int>{1, 3, 5};
  c2 = std::move(*c);
  delete c;
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-assignment works
  // Wrap in a lambda to silence a compiler warning about this.
  constexpr auto self_assign = [](auto& x, auto&& y) {
    x = std::forward<decltype(y)>(y);
  };
  self_assign(c2, c2);
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, swap_member_fun) {
  countset<int> c1{1, 3, 5};
  countset<int> c2{2, 4, 6};
  c1.swap(c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  c1.swap(c1);
  assert_countset_is(c1, {2, 4, 6});
}

TEST(countset, swap_hidden_friend) {
  countset<int> c1{1, 3, 5};
  countset<int> c2{2, 4, 6};
  swap(c1, c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  swap(c1, c1);
  assert_countset_is(c1, {2, 4, 6});
}

TEST(countset, std_swap) {
  countset<int> c1{1, 3, 5};
  countset<int> c2{2, 4, 6};
  // Note: This is less efficient than the hidden friend implementation, prefer
  // using the insane pattern of "using std::swap; swap(a, b);".
  std::swap(c1, c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  std::swap(c1, c1);
  assert_countset_is(c1, {2, 4, 6});
}

TEST(countset, basic_getters) {
  countset<int> c{1, 3, 5};
  ASSERT_FALSE(c.empty());
  ASSERT_EQ(c.size(), 3);
  ASSERT_EQ(c.max_size(), std::numeric_limits<std::size_t>::max());
  static_assert(
      std::is_same_v<decltype(c.get_allocator()), decltype(c)::allocator_type>);
  static_assert(
      std::is_same_v<decltype(c.key_comp()), decltype(c)::key_compare>);
  static_assert(
      std::is_same_v<decltype(c.value_comp()), decltype(c)::value_compare>);
  static_assert(std::is_same_v<decltype(c.get_rng()), decltype(c)::rng_type>);
}

TEST(countset, iterator_types) {

  countset<int> c{};

  static_assert(std::is_same_v<decltype(c)::iterator::iterator_category,
                               std::bidirectional_iterator_tag>);
  static_assert(
      std::is_same_v<decltype(c)::iterator::difference_type, std::ptrdiff_t>);
  static_assert(std::is_same_v<decltype(c)::iterator::value_type, const int>);
  static_assert(std::is_same_v<decltype(c)::iterator::reference, const int&>);
  static_assert(std::is_same_v<decltype(c)::iterator::pointer, const int*>);

  ASSERT_EQ(c.begin(), c.end());
  static_assert(std::is_same_v<decltype(c.begin()), decltype(c)::iterator>);
  static_assert(std::is_same_v<decltype(c.end()), decltype(c)::iterator>);

  ASSERT_EQ(c.cbegin(), c.cend());
  static_assert(
      std::is_same_v<decltype(c.cbegin()), decltype(c)::const_iterator>);
  static_assert(
      std::is_same_v<decltype(c.cend()), decltype(c)::const_iterator>);

  ASSERT_EQ(c.rbegin(), c.rend());
  static_assert(
      std::is_same_v<decltype(c.rbegin()), decltype(c)::reverse_iterator>);
  static_assert(
      std::is_same_v<decltype(c.rend()), decltype(c)::reverse_iterator>);

  ASSERT_EQ(c.crbegin(), c.crend());
  static_assert(std::is_same_v<decltype(c.crbegin()),
                               decltype(c)::const_reverse_iterator>);
  static_assert(
      std::is_same_v<decltype(c.crend()), decltype(c)::const_reverse_iterator>);
}

TEST(countset, iterator_base) {
  countset<int> c{1};
  ASSERT_NE(c.begin(), c.end());
  ASSERT_EQ(std::next(c.begin()), c.end());
  ASSERT_EQ(std::next(c.begin()).base(), c.end());

  ASSERT_NE(c.cbegin(), c.cend());
  ASSERT_EQ(std::next(c.cbegin()), c.cend());
  ASSERT_EQ(std::next(c.cbegin()).base(), c.cend());

  ASSERT_NE(c.rbegin(), c.rend());
  ASSERT_EQ(std::next(c.rbegin()), c.rend());
  ASSERT_EQ(c.rbegin().base(), c.begin());

  ASSERT_NE(c.crbegin(), c.crend());
  ASSERT_EQ(std::next(c.crbegin()), c.rend());
  ASSERT_EQ(c.crbegin().base(), c.cbegin());
}

TEST(countset, iterator_copy_move) {
  countset<int> c{1};
  auto it = c.begin();
  ASSERT_EQ(it, c.begin());
  auto it2(it);
  ASSERT_EQ(it2, c.begin());
  auto it3(std::move(it));  // NOLINT(performance-move-const-arg)
  ASSERT_EQ(it3, c.begin());
  it3 = c.end();
  ASSERT_EQ(it3, c.end());
  it2 = std::move(it3);  // NOLINT(performance-move-const-arg)
  ASSERT_EQ(it2, c.end());
}

TEST(countset, iterator_increment_decrement_operators) {
  countset<int> c{1, 3, 5};

  {
    auto it = c.begin();
    auto it2 = it++;
    ASSERT_EQ(it, std::next(c.begin()));
    ASSERT_EQ(it2, c.begin());
  }

  {
    auto it = c.begin();
    auto it2 = ++it;
    ASSERT_EQ(it, std::next(c.begin()));
    ASSERT_EQ(it2, std::next(c.begin()));
  }

  {
    auto it = std::next(c.begin());
    auto it2 = it--;
    ASSERT_EQ(it, c.begin());
    ASSERT_EQ(it2, std::next(c.begin()));
  }

  {
    auto it = std::next(c.begin());
    auto it2 = --it;
    ASSERT_EQ(it, c.begin());
    ASSERT_EQ(it2, c.begin());
  }
}

TEST(countset, iterator_increment_decrement_operators_reverse) {
  countset<int> c{1, 3, 5};

  {
    auto it = c.rbegin();
    auto it2 = it++;
    ASSERT_EQ(it, std::next(c.rbegin()));
    ASSERT_EQ(it2, c.rbegin());
  }

  {
    auto it = c.rbegin();
    auto it2 = ++it;
    ASSERT_EQ(it, std::next(c.rbegin()));
    ASSERT_EQ(it2, std::next(c.rbegin()));
  }

  {
    auto it = std::next(c.rbegin());
    auto it2 = it--;
    ASSERT_EQ(it, c.rbegin());
    ASSERT_EQ(it2, std::next(c.rbegin()));
  }

  {
    auto it = std::next(c.rbegin());
    auto it2 = --it;
    ASSERT_EQ(it, c.rbegin());
    ASSERT_EQ(it2, c.rbegin());
  }
}

TEST(countset, iterator_star_arrow_operators) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {5, 6}};

  auto it = c.begin();
  ASSERT_EQ(*it, (wrapped_ints{1, 2}));
  ASSERT_EQ(it->value, 1);
  ASSERT_EQ(it->second_value, 2);
  it++;
  ASSERT_EQ(*it, (wrapped_ints{3, 4}));
  ASSERT_EQ(it->value, 3);
  ASSERT_EQ(it->second_value, 4);
  it++;
  ASSERT_EQ(*it, (wrapped_ints{5, 6}));
  ASSERT_EQ(it->value, 5);
  ASSERT_EQ(it->second_value, 6);
}

TEST(countset, insert_copy) {
  countset<int> c{1, 3, 5};
  {
    const int val = 6;
    auto result = c.insert(val);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(*result.first, val);
    ASSERT_EQ(result.first, std::next(c.begin(), 3));
    assert_countset_is(c, {1, 3, 5, 6});
  }
  {
    const int val = 0;
    auto result = c.insert(val);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(*result.first, val);
    ASSERT_EQ(result.first, c.begin());
    assert_countset_is(c, {0, 1, 3, 5, 6});
  }
  {
    const int val = 4;
    auto result = c.insert(val);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(*result.first, val);
    ASSERT_EQ(result.first, std::next(c.begin(), 3));
    assert_countset_is(c, {0, 1, 3, 4, 5, 6});
  }
  {
    const int val = 4;
    auto result = c.insert(val);
    ASSERT_FALSE(result.second);
    ASSERT_EQ(*result.first, val);
    ASSERT_EQ(result.first, std::next(c.begin(), 3));
    assert_countset_is(c, {0, 1, 3, 4, 5, 6});
  }
}

TEST(countset, insert_copy_hint) {
  countset<int> c{1, 3, 5};
  {
    const int val = 6;
    auto result = c.insert(c.end(), val);
    ASSERT_EQ(*result, val);
    ASSERT_EQ(result, std::next(c.begin(), 3));
    assert_countset_is(c, {1, 3, 5, 6});
  }
  {
    const int val = 0;
    auto result = c.insert(c.begin(), val);
    ASSERT_EQ(*result, val);
    ASSERT_EQ(result, c.begin());
    assert_countset_is(c, {0, 1, 3, 5, 6});
  }
  {
    const int val = 4;
    auto result = c.insert(std::next(c.begin(), 3), val);
    ASSERT_EQ(*result, val);
    ASSERT_EQ(result, std::next(c.begin(), 3));
    assert_countset_is(c, {0, 1, 3, 4, 5, 6});
  }
  {
    const int val = 4;
    auto result = c.insert(std::next(c.begin(), 3), val);
    ASSERT_EQ(*result, val);
    ASSERT_EQ(result, std::next(c.begin(), 3));
    assert_countset_is(c, {0, 1, 3, 4, 5, 6});
  }
}

TEST(countset, insert_move) {
  countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  counted_ops::clear_counters();
  auto result = c.insert(counted_ops(4));
  ASSERT_EQ(counted_ops::copy_ctor_count, 0);
  ASSERT_EQ(counted_ops::copy_assign_count, 0);
  ASSERT_TRUE(result.second);
  ASSERT_EQ(*result.first, counted_ops(4));
  ASSERT_EQ(result.first, std::next(c.begin(), 2));
}

TEST(countset, insert_move_hint) {
  countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  counted_ops::clear_counters();
  auto result = c.insert(std::next(c.begin(), 2), counted_ops(4));
  ASSERT_EQ(counted_ops::copy_ctor_count, 0);
  ASSERT_EQ(counted_ops::copy_assign_count, 0);
  ASSERT_EQ(*result, counted_ops(4));
  ASSERT_EQ(result, std::next(c.begin(), 2));
}

TEST(countset, insert_initializer_list) {
  countset<int> c{1, 3, 5, 7, 9};
  c.insert({2, 5, 4, 9, 10, 6});
  assert_countset_is(c, {1, 2, 3, 4, 5, 6, 7, 9, 10});
}

TEST(countset, insert_iterator_pair) {
  countset<int> c{1, 3, 5, 7, 9};
  std::vector<int> to_insert{2, 5, 4, 9, 10, 6};
  c.insert(to_insert.begin(), to_insert.end());
  assert_countset_is(c, {1, 2, 3, 4, 5, 6, 7, 9, 10});
}

TEST(countset, emplace) {
  countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  {
    counted_ops::clear_counters();
    auto result = c.emplace(4);
    ASSERT_EQ(counted_ops::default_ctor_count, 0);
    ASSERT_EQ(counted_ops::value_ctor_count, 1);
    ASSERT_EQ(counted_ops::copy_ctor_count, 0);
    ASSERT_EQ(counted_ops::move_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_assign_count, 0);
    ASSERT_EQ(counted_ops::move_assign_count, 0);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(*result.first, counted_ops(4));
    ASSERT_EQ(result.first, std::next(c.begin(), 2));
  }
  {
    counted_ops::clear_counters();
    auto result = c.emplace();
    ASSERT_EQ(counted_ops::default_ctor_count, 1);
    ASSERT_EQ(counted_ops::value_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_ctor_count, 0);
    ASSERT_EQ(counted_ops::move_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_assign_count, 0);
    ASSERT_EQ(counted_ops::move_assign_count, 0);
    ASSERT_TRUE(result.second);
    ASSERT_EQ(*result.first, counted_ops());
    ASSERT_EQ(result.first, c.begin());
  }
}

TEST(countset, emplace_hint) {
  countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  {
    counted_ops::clear_counters();
    auto result = c.emplace_hint(std::next(c.begin(), 2), 4);
    ASSERT_EQ(counted_ops::default_ctor_count, 0);
    ASSERT_EQ(counted_ops::value_ctor_count, 1);
    ASSERT_EQ(counted_ops::copy_ctor_count, 0);
    ASSERT_EQ(counted_ops::move_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_assign_count, 0);
    ASSERT_EQ(counted_ops::move_assign_count, 0);
    ASSERT_EQ(*result, counted_ops(4));
    ASSERT_EQ(result, std::next(c.begin(), 2));
  }
  {
    counted_ops::clear_counters();
    auto result = c.emplace_hint(c.begin());
    ASSERT_EQ(counted_ops::default_ctor_count, 1);
    ASSERT_EQ(counted_ops::value_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_ctor_count, 0);
    ASSERT_EQ(counted_ops::move_ctor_count, 0);
    ASSERT_EQ(counted_ops::copy_assign_count, 0);
    ASSERT_EQ(counted_ops::move_assign_count, 0);
    ASSERT_EQ(*result, counted_ops());
    ASSERT_EQ(result, c.begin());
  }
}

TEST(countset, erase_iterator) {
  countset<int> c{1, 3, 5, 7, 9};
  {
    auto result = c.erase(std::next(c.begin(), 3));
    ASSERT_EQ(*result, 9);
    ASSERT_EQ(result, std::next(c.begin(), 3));
    assert_countset_is(c, {1, 3, 5, 9});
  }
  {
    auto result = c.erase(std::next(c.begin(), 3));
    ASSERT_EQ(result, c.end());
    assert_countset_is(c, {1, 3, 5});
  }
  {
    auto result = c.erase(c.begin());
    ASSERT_EQ(result, c.begin());
    assert_countset_is(c, {3, 5});
  }
}

TEST(countset, erase_range) {
  countset<int> c{1, 3, 5, 7, 9, 11, 13, 15};
  {
    auto result = c.erase(std::next(c.begin(), 2), std::next(c.begin(), 3));
    ASSERT_EQ(*result, 7);
    ASSERT_EQ(result, std::next(c.begin(), 2));
    assert_countset_is(c, {1, 3, 7, 9, 11, 13, 15});
  }
  {
    auto result = c.erase(std::next(c.begin(), 4), c.end());
    ASSERT_EQ(result, c.end());
    assert_countset_is(c, {1, 3, 7, 9});
  }
  {
    auto result = c.erase(c.begin(), c.begin());
    ASSERT_EQ(result, c.begin());
    assert_countset_is(c, {1, 3, 7, 9});
  }
  {
    auto result = c.erase(c.begin(), std::next(c.begin(), 3));
    ASSERT_EQ(result, c.begin());
    assert_countset_is(c, {9});
  }
}

TEST(countset, erase_value) {
  countset<int> c{1, 3, 5, 7, 9};
  {
    auto result = c.erase(7);
    ASSERT_EQ(result, 1);
    assert_countset_is(c, {1, 3, 5, 9});
  }
  {
    auto result = c.erase(0);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {1, 3, 5, 9});
  }
  {
    auto result = c.erase(10);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {1, 3, 5, 9});
  }
  {
    auto result = c.erase(4);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {1, 3, 5, 9});
  }
  {
    auto result = c.erase(1);
    ASSERT_EQ(result, 1);
    assert_countset_is(c, {3, 5, 9});
  }
  {
    auto result = c.erase(9);
    ASSERT_EQ(result, 1);
    assert_countset_is(c, {3, 5});
  }
}

TEST(countset, erase_transparent_value) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  {
    auto result = c.erase(5);
    ASSERT_EQ(result, 1);
    assert_countset_is(c, {{1, 2}, {3, 4}, {3, 6}, {3, 8}, {7, 6}});
  }
  {
    auto result = c.erase(4);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {{1, 2}, {3, 4}, {3, 6}, {3, 8}, {7, 6}});
  }
  {
    auto result = c.erase(8);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {{1, 2}, {3, 4}, {3, 6}, {3, 8}, {7, 6}});
  }
  {
    auto result = c.erase(0);
    ASSERT_EQ(result, 0);
    assert_countset_is(c, {{1, 2}, {3, 4}, {3, 6}, {3, 8}, {7, 6}});
  }
  {
    auto result = c.erase(3);
    ASSERT_EQ(result, 3);
    assert_countset_is(c, {{1, 2}, {7, 6}});
  }
}

TEST(countset, clear) {
  countset<int> c1{1, 3, 5};
  c1.clear();
  assert_countset_is(c1, {});
  c1.insert({2, 4, 6});
  assert_countset_is(c1, {2, 4, 6});
  c1.clear();
  assert_countset_is(c1, {});
  c1.clear();
  assert_countset_is(c1, {});
}

TEST(countset, count) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_EQ(c.count(0), 0);
  ASSERT_EQ(c.count(1), 1);
  ASSERT_EQ(c.count(2), 0);
  ASSERT_EQ(c.count(3), 1);
  ASSERT_EQ(c.count(4), 0);
  ASSERT_EQ(c.count(5), 1);
  ASSERT_EQ(c.count(6), 0);
  ASSERT_EQ(c.count(7), 1);
  ASSERT_EQ(c.count(8), 0);
}

TEST(countset, count_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_EQ(c.count(0), 0);
  ASSERT_EQ(c.count(1), 1);
  ASSERT_EQ(c.count(2), 0);
  ASSERT_EQ(c.count(3), 3);
  ASSERT_EQ(c.count(4), 0);
  ASSERT_EQ(c.count(5), 1);
  ASSERT_EQ(c.count(6), 0);
  ASSERT_EQ(c.count(7), 1);
  ASSERT_EQ(c.count(8), 0);
}

TEST(countset, contains) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_FALSE(c.contains(0));
  ASSERT_TRUE(c.contains(1));
  ASSERT_FALSE(c.contains(2));
  ASSERT_TRUE(c.contains(3));
  ASSERT_FALSE(c.contains(4));
  ASSERT_TRUE(c.contains(5));
  ASSERT_FALSE(c.contains(6));
  ASSERT_TRUE(c.contains(7));
  ASSERT_FALSE(c.contains(8));
}

TEST(countset, contains_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_FALSE(c.contains(0));
  ASSERT_TRUE(c.contains(1));
  ASSERT_FALSE(c.contains(2));
  ASSERT_TRUE(c.contains(3));
  ASSERT_FALSE(c.contains(4));
  ASSERT_TRUE(c.contains(5));
  ASSERT_FALSE(c.contains(6));
  ASSERT_TRUE(c.contains(7));
  ASSERT_FALSE(c.contains(8));
}

TEST(countset, find_in_empty_set) {
  countset<int> c{};
  ASSERT_EQ(c.find(0), c.end());
}

TEST(countset, find) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_EQ(c.find(0), c.end());
  ASSERT_EQ(c.find(1), std::next(c.begin(), 0));
  ASSERT_EQ(c.find(2), c.end());
  ASSERT_EQ(c.find(3), std::next(c.begin(), 1));
  ASSERT_EQ(c.find(4), c.end());
  ASSERT_EQ(c.find(5), std::next(c.begin(), 2));
  ASSERT_EQ(c.find(6), c.end());
  ASSERT_EQ(c.find(7), std::next(c.begin(), 3));
  ASSERT_EQ(c.find(8), c.end());
}

TEST(countset, find_transparent_in_empty_set) {
  countset<wrapped_ints> c{};
  ASSERT_EQ(c.find(0), c.end());
}

TEST(countset, find_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_EQ(c.find(0), c.end());
  ASSERT_EQ(c.find(1), std::next(c.begin(), 0));
  ASSERT_EQ(c.find(2), c.end());
  ASSERT_EQ(c.find(3), std::next(c.begin(), 1));
  ASSERT_EQ(c.find(4), c.end());
  ASSERT_EQ(c.find(5), std::next(c.begin(), 4));
  ASSERT_EQ(c.find(6), c.end());
  ASSERT_EQ(c.find(7), std::next(c.begin(), 5));
  ASSERT_EQ(c.find(8), c.end());
}

TEST(countset, lower_bound_in_empty_set) {
  countset<int> c{};
  ASSERT_EQ(c.lower_bound(0), c.end());
}

TEST(countset, lower_bound) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_EQ(c.lower_bound(0), std::next(c.begin(), 0));
  ASSERT_EQ(c.lower_bound(1), std::next(c.begin(), 0));
  ASSERT_EQ(c.lower_bound(2), std::next(c.begin(), 1));
  ASSERT_EQ(c.lower_bound(3), std::next(c.begin(), 1));
  ASSERT_EQ(c.lower_bound(4), std::next(c.begin(), 2));
  ASSERT_EQ(c.lower_bound(5), std::next(c.begin(), 2));
  ASSERT_EQ(c.lower_bound(6), std::next(c.begin(), 3));
  ASSERT_EQ(c.lower_bound(7), std::next(c.begin(), 3));
  ASSERT_EQ(c.lower_bound(8), c.end());
}

TEST(countset, lower_bound_transparent_in_empty_set) {
  countset<wrapped_ints> c{};
  ASSERT_EQ(c.lower_bound(0), c.end());
}

TEST(countset, lower_bound_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_EQ(c.lower_bound(0), std::next(c.begin(), 0));
  ASSERT_EQ(c.lower_bound(1), std::next(c.begin(), 0));
  ASSERT_EQ(c.lower_bound(2), std::next(c.begin(), 1));
  ASSERT_EQ(c.lower_bound(3), std::next(c.begin(), 1));
  ASSERT_EQ(c.lower_bound(4), std::next(c.begin(), 4));
  ASSERT_EQ(c.lower_bound(5), std::next(c.begin(), 4));
  ASSERT_EQ(c.lower_bound(6), std::next(c.begin(), 5));
  ASSERT_EQ(c.lower_bound(7), std::next(c.begin(), 5));
  ASSERT_EQ(c.lower_bound(8), c.end());
}

TEST(countset, upper_bound_in_empty_set) {
  countset<int> c{};
  ASSERT_EQ(c.upper_bound(0), c.end());
}

TEST(countset, upper_bound) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_EQ(c.upper_bound(0), std::next(c.begin(), 0));
  ASSERT_EQ(c.upper_bound(1), std::next(c.begin(), 1));
  ASSERT_EQ(c.upper_bound(2), std::next(c.begin(), 1));
  ASSERT_EQ(c.upper_bound(3), std::next(c.begin(), 2));
  ASSERT_EQ(c.upper_bound(4), std::next(c.begin(), 2));
  ASSERT_EQ(c.upper_bound(5), std::next(c.begin(), 3));
  ASSERT_EQ(c.upper_bound(6), std::next(c.begin(), 3));
  ASSERT_EQ(c.upper_bound(7), c.end());
  ASSERT_EQ(c.upper_bound(8), c.end());
}

TEST(countset, upper_bound_transparent_in_empty_set) {
  countset<wrapped_ints> c{};
  ASSERT_EQ(c.upper_bound(0), c.end());
}

TEST(countset, upper_bound_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_EQ(c.upper_bound(0), std::next(c.begin(), 0));
  ASSERT_EQ(c.upper_bound(1), std::next(c.begin(), 1));
  ASSERT_EQ(c.upper_bound(2), std::next(c.begin(), 1));
  ASSERT_EQ(c.upper_bound(3), std::next(c.begin(), 4));
  ASSERT_EQ(c.upper_bound(4), std::next(c.begin(), 4));
  ASSERT_EQ(c.upper_bound(5), std::next(c.begin(), 5));
  ASSERT_EQ(c.upper_bound(6), std::next(c.begin(), 5));
  ASSERT_EQ(c.upper_bound(7), c.end());
  ASSERT_EQ(c.upper_bound(8), c.end());
}

TEST(countset, equal_range_in_empty_set) {
  countset<int> c{};
  ASSERT_EQ(c.equal_range(0), std::pair(c.end(), c.end()));
}

TEST(countset, equal_range) {
  countset<int> c{1, 3, 5, 7};
  ASSERT_EQ(c.equal_range(0),
            std::pair(std::next(c.begin(), 0), std::next(c.begin(), 0)));
  ASSERT_EQ(c.equal_range(1),
            std::pair(std::next(c.begin(), 0), std::next(c.begin(), 1)));
  ASSERT_EQ(c.equal_range(2),
            std::pair(std::next(c.begin(), 1), std::next(c.begin(), 1)));
  ASSERT_EQ(c.equal_range(3),
            std::pair(std::next(c.begin(), 1), std::next(c.begin(), 2)));
  ASSERT_EQ(c.equal_range(4),
            std::pair(std::next(c.begin(), 2), std::next(c.begin(), 2)));
  ASSERT_EQ(c.equal_range(5),
            std::pair(std::next(c.begin(), 2), std::next(c.begin(), 3)));
  ASSERT_EQ(c.equal_range(6),
            std::pair(std::next(c.begin(), 3), std::next(c.begin(), 3)));
  ASSERT_EQ(c.equal_range(7), std::pair(std::next(c.begin(), 3), c.end()));
  ASSERT_EQ(c.equal_range(8), std::pair(c.end(), c.end()));
}

TEST(countset, equal_range_transparent_in_empty_set) {
  countset<wrapped_ints> c{};
  ASSERT_EQ(c.equal_range(0), std::pair(c.end(), c.end()));
}

TEST(countset, equal_range_transparent) {
  countset<wrapped_ints> c{{1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
  ASSERT_EQ(c.equal_range(0),
            std::pair(std::next(c.begin(), 0), std::next(c.begin(), 0)));
  ASSERT_EQ(c.equal_range(1),
            std::pair(std::next(c.begin(), 0), std::next(c.begin(), 1)));
  ASSERT_EQ(c.equal_range(2),
            std::pair(std::next(c.begin(), 1), std::next(c.begin(), 1)));
  ASSERT_EQ(c.equal_range(3),
            std::pair(std::next(c.begin(), 1), std::next(c.begin(), 4)));
  ASSERT_EQ(c.equal_range(4),
            std::pair(std::next(c.begin(), 4), std::next(c.begin(), 4)));
  ASSERT_EQ(c.equal_range(5),
            std::pair(std::next(c.begin(), 4), std::next(c.begin(), 5)));
  ASSERT_EQ(c.equal_range(6),
            std::pair(std::next(c.begin(), 5), std::next(c.begin(), 5)));
  ASSERT_EQ(c.equal_range(7), std::pair(std::next(c.begin(), 5), c.end()));
  ASSERT_EQ(c.equal_range(8), std::pair(c.end(), c.end()));
}

struct traced_comparator_storage {
  std::size_t num_compares = 0;
};

static traced_comparator_storage* default_cmp_storage() noexcept;

template<class T>
struct traced_comparator {
  traced_comparator_storage* i = default_cmp_storage();

  // Not transparent
  [[nodiscard]] bool operator()(const T& a, const T& b) const noexcept {
    i->num_compares++;
    return a < b;
  }

  [[nodiscard]] bool operator==(const traced_comparator&) const noexcept
      = default;
};

struct traced_allocator_storage {
  std::byte data[4096]{};
  std::byte* end = data;
  std::byte* end_capacity = data + 4096;
  std::set<std::pair<std::byte*, std::size_t>> allocs;
  std::set<std::pair<std::byte*, std::size_t>> deallocs;
};

static traced_allocator_storage* default_allocator_storage() noexcept;

template<class T>
struct traced_allocator {
  using size_type [[maybe_unused]] = std::size_t;
  using difference_type [[maybe_unused]] = std::ptrdiff_t;
  using value_type [[maybe_unused]] = T;
  using propagate_on_container_move_assignment [[maybe_unused]]
  = std::false_type;
  using is_always_equal [[maybe_unused]] = std::true_type;

  static inline constexpr std::size_t align = alignof(std::max_align_t);
  static_assert(alignof(T) <= align);

  static constexpr std::size_t alloc_size(std::size_t n) noexcept {
    return (n * sizeof(T) + align - 1) / align * align;
  }

  traced_allocator_storage* i = default_allocator_storage();

  traced_allocator() noexcept = default;

  explicit traced_allocator(traced_allocator_storage* i) noexcept: i(i) {
  }

  template<class U>
  explicit traced_allocator(const traced_allocator<U>& a) noexcept: i(a.i) {
  }

  T* allocate(std::size_t n) {
    EXPECT_EQ(n, 1);
    const auto size = alloc_size(n);
    if (i->end + size > i->end_capacity) {
      throw std::bad_alloc();
    }
    auto t = i->end;
    i->end += size;
    i->allocs.emplace(t, size);
    return reinterpret_cast<T*>(t);
  }

  void deallocate(T* p, std::size_t n) noexcept {
    EXPECT_EQ(n, 1);
    i->deallocs.emplace(reinterpret_cast<std::byte*>(p), alloc_size(n));
  }

  [[nodiscard]] bool operator==(const traced_allocator&) const noexcept
      = default;
};

struct traced_rng_storage {
  int num_calls = 0;
  default_rng r;
};

static traced_rng_storage* default_rng_storage() noexcept;

struct traced_rng {
  traced_rng_storage* i = default_rng_storage();

  [[nodiscard]] std::size_t operator()() const noexcept {
    i->num_calls++;
    return i->r();
  }

  [[nodiscard]] bool operator==(const traced_rng&) const noexcept = default;
};

// Ensure all functions are instantiated, so we have accurate coverage
// information.
template struct eeds::
    countset<int, traced_comparator<int>, traced_allocator<int>, traced_rng>;

struct countset_traits : ::testing::Test {
  using traced_set = countset<int,
                              traced_comparator<int>,
                              traced_allocator<int>,
                              traced_rng>;

  static inline traced_comparator_storage* default_cmp = nullptr;
  static inline traced_allocator_storage* default_alloc = nullptr;
  static inline traced_rng_storage* default_rng_storage = nullptr;

  traced_comparator_storage cmp_storage{};
  traced_allocator_storage alloc_storage{};
  traced_rng_storage rng_storage{};

  countset_traits() {
    default_cmp = new traced_comparator_storage();
    default_alloc = new traced_allocator_storage();
    default_rng_storage = new traced_rng_storage();
  }
  ~countset_traits() override {
    delete default_cmp;
    delete default_alloc;
    delete default_rng_storage;
  }

  void check_uses_default_cmp(traced_set& c) {
    check_uses_cmp_internal(c, *default_cmp, cmp_storage);
  }

  static void check_uses_cmp(traced_set& c,
                             traced_comparator_storage& used_cmp) {
    check_uses_cmp_internal(c, used_cmp, *default_cmp);
  }

  static void check_uses_cmp_internal(traced_set& c,
                                      traced_comparator_storage& used_cmp,
                                      traced_comparator_storage& other_cmp) {
    EXPECT_NE(c.key_comp(), traced_comparator<int>{&other_cmp});
    EXPECT_EQ(c.key_comp(), traced_comparator<int>{&used_cmp});
    EXPECT_NE(c.value_comp(), traced_comparator<int>{&other_cmp});
    EXPECT_EQ(c.value_comp(), traced_comparator<int>{&used_cmp});
    auto prev_num_comps = used_cmp.num_compares;
    auto prev_num_comps_other = other_cmp.num_compares;
    c.find(4);
    EXPECT_GT(used_cmp.num_compares, prev_num_comps);
    EXPECT_EQ(other_cmp.num_compares, prev_num_comps_other);
  }

  void check_uses_default_allocator(traced_set& c) {
    check_uses_allocator_impl(c, *default_alloc, alloc_storage);
  }

  static void check_uses_allocator(traced_set& c,
                                   traced_allocator_storage& other_alloc) {
    check_uses_allocator_impl(c, other_alloc, *default_alloc);
  }

  static void check_uses_allocator_impl(traced_set& c,
                                        traced_allocator_storage& used_alloc,
                                        traced_allocator_storage& other_alloc) {
    EXPECT_EQ(c.get_allocator(), traced_allocator<int>{&used_alloc});
    EXPECT_NE(c.get_allocator(), traced_allocator<int>{&other_alloc});

    auto prev_num_allocations = used_alloc.allocs.size();
    auto prev_num_allocations_other = other_alloc.allocs.size();
    c.insert(4);
    EXPECT_EQ(other_alloc.allocs.size(), prev_num_allocations_other);
    EXPECT_EQ(used_alloc.allocs.size(), prev_num_allocations + 1);
  }

  void check_uses_default_rng(traced_set& c) {
    check_uses_rng_impl(c, *default_rng_storage, rng_storage);
  }

  static void check_uses_rng(traced_set& c, traced_rng_storage& other_rng) {
    check_uses_rng_impl(c, other_rng, *default_rng_storage);
  }

  static void check_uses_rng_impl(traced_set& c,
                                  traced_rng_storage& used_rng,
                                  traced_rng_storage& other_rng) {
    EXPECT_EQ(c.get_rng(), traced_rng{&used_rng});
    EXPECT_NE(c.get_rng(), traced_rng{&other_rng});

    auto prev_num_rng_calls = used_rng.num_calls;
    auto prev_num_rng_calls_other = other_rng.num_calls;
    c.insert(4);
    EXPECT_EQ(other_rng.num_calls, prev_num_rng_calls_other);
    EXPECT_EQ(used_rng.num_calls, prev_num_rng_calls + 1);
  }
};

traced_comparator_storage* default_cmp_storage() noexcept {
  return countset_traits::default_cmp;
}
traced_allocator_storage* default_allocator_storage() noexcept {
  return countset_traits::default_alloc;
}
traced_rng_storage* default_rng_storage() noexcept {
  return countset_traits::default_rng_storage;
}

TEST_F(countset_traits, default_ctor) {
  traced_set c;
  c.insert({1, 3, 5});

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, default_ctor_C) {
  traced_set c(traced_comparator<int>{&cmp_storage});
  c.insert({1, 3, 5});

  check_uses_cmp(c, cmp_storage);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, default_ctor_C_A) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage});
  c.insert({1, 3, 5});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, default_ctor_C_A_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, default_ctor_A) {
  traced_set c(traced_allocator<int>{&alloc_storage});
  c.insert({1, 3, 5});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, default_ctor_A_R) {
  traced_set c(traced_allocator<int>{&alloc_storage}, traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, default_ctor_R) {
  traced_set c(traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_rng(c, rng_storage);
}

TEST_F(countset_traits, copy_ctor) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_set c2(c);  // NOLINT(performance-unnecessary-copy-initialization)
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, copy_ctor_C) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_set c2(c, traced_comparator<int>{&other_cmp});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, copy_ctor_C_A) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_allocator_storage other_alloc{};
  traced_set c2(c,
                traced_comparator<int>{&other_cmp},
                traced_allocator<int>{&other_alloc});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, copy_ctor_C_A_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_allocator_storage other_alloc{};
  traced_rng_storage other_rng{};
  traced_set c2(c,
                traced_comparator<int>{&other_cmp},
                traced_allocator<int>{&other_alloc},
                traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, other_rng);
}
TEST_F(countset_traits, copy_ctor_A) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_allocator_storage other_alloc{};
  traced_set c2(c, traced_allocator<int>{&other_alloc});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, copy_ctor_A_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_allocator_storage other_alloc{};
  traced_rng_storage other_rng{};
  traced_set c2(c, traced_allocator<int>{&other_alloc}, traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, other_rng);
}
TEST_F(countset_traits, copy_ctor_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_rng_storage other_rng{};
  traced_set c2(c, traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, other_rng);
}

TEST_F(countset_traits, move_ctor) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_set c2(std::move(c));
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, move_ctor_C) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_set c2(std::move(c), traced_comparator<int>{&other_cmp});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, move_ctor_C_A) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_allocator_storage other_alloc{};
  traced_set c2(std::move(c),
                traced_comparator<int>{&other_cmp},
                traced_allocator<int>{&other_alloc});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, move_ctor_C_A_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_comparator_storage other_cmp{};
  traced_allocator_storage other_alloc{};
  traced_rng_storage other_rng{};
  traced_set c2(std::move(c),
                traced_comparator<int>{&other_cmp},
                traced_allocator<int>{&other_alloc},
                traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, other_cmp);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, other_rng);
}
TEST_F(countset_traits, move_ctor_A) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_allocator_storage other_alloc{};
  traced_set c2(std::move(c), traced_allocator<int>{&other_alloc});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, rng_storage);
}
TEST_F(countset_traits, move_ctor_A_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_allocator_storage other_alloc{};
  traced_rng_storage other_rng{};
  traced_set c2(std::move(c),
                traced_allocator<int>{&other_alloc},
                traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, other_alloc);
  check_uses_rng(c2, other_rng);
}
TEST_F(countset_traits, move_ctor_R) {
  traced_set c(traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});
  c.insert({1, 3, 5});

  traced_rng_storage other_rng{};
  traced_set c2(std::move(c), traced_rng{&other_rng});
  assert_countset_is(c2, {1, 3, 5});
  check_uses_cmp(c2, cmp_storage);
  check_uses_allocator(c2, alloc_storage);
  check_uses_rng(c2, other_rng);
}

TEST_F(countset_traits, iterator_pair_ctor) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(), v.end());

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, iterator_pair_ctor_C) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(), v.end(), traced_comparator<int>{&cmp_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, iterator_pair_ctor_C_A) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(),
               v.end(),
               traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, iterator_pair_ctor_C_A_R) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(),
               v.end(),
               traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, iterator_pair_ctor_A) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(), v.end(), traced_allocator<int>{&alloc_storage});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, iterator_pair_ctor_A_R) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(),
               v.end(),
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, iterator_pair_ctor_R) {
  std::vector<int> v{1, 3, 5};
  traced_set c(v.begin(), v.end(), traced_rng{&rng_storage});

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_rng(c, rng_storage);
}

TEST_F(countset_traits, init_list_ctor) {
  traced_set c({1, 3, 5});

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, init_list_ctor_C) {
  traced_set c({1, 3, 5}, traced_comparator<int>{&cmp_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_default_allocator(c);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, init_list_ctor_C_A) {
  traced_set c({1, 3, 5},
               traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, init_list_ctor_C_A_R) {
  traced_set c({1, 3, 5},
               traced_comparator<int>{&cmp_storage},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});

  check_uses_cmp(c, cmp_storage);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, init_list_ctor_A) {
  traced_set c({1, 3, 5}, traced_allocator<int>{&alloc_storage});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_default_rng(c);
}
TEST_F(countset_traits, init_list_ctor_A_R) {
  traced_set c({1, 3, 5},
               traced_allocator<int>{&alloc_storage},
               traced_rng{&rng_storage});

  check_uses_default_cmp(c);
  check_uses_allocator(c, alloc_storage);
  check_uses_rng(c, rng_storage);
}
TEST_F(countset_traits, init_list_ctor_R) {
  traced_set c({1, 3, 5}, traced_rng{&rng_storage});

  check_uses_default_cmp(c);
  check_uses_default_allocator(c);
  check_uses_rng(c, rng_storage);
}

// Just a different group
struct countset_malloc_checks : countset_traits {};

TEST_F(countset_malloc_checks, memory_is_cleared_in_clear) {
  traced_set c{1, 3, 5};
  c.clear();
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

TEST_F(countset_malloc_checks, memory_is_cleared_in_dtor) {
  { traced_set c{1, 3, 5}; }
  EXPECT_EQ(default_alloc->allocs.size(), 3);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

TEST_F(countset_malloc_checks, memory_is_deallocated_in_erase) {
  traced_set c{1, 3, 5};
  EXPECT_EQ(default_alloc->allocs.size(), 3);
  EXPECT_TRUE(default_alloc->deallocs.empty());
  c.erase(3);
  EXPECT_EQ(default_alloc->deallocs.size(), 1);
  EXPECT_TRUE(default_alloc->allocs.contains(*default_alloc->deallocs.begin()));
  c.erase(1);
  EXPECT_EQ(default_alloc->deallocs.size(), 2);
  c.erase(5);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

TEST_F(countset_malloc_checks, no_leak_when_allocation_fails_mid_ctor) {
  default_alloc->end = default_alloc->end_capacity
      - 3 * traced_allocator<traced_set::node_type>::alloc_size(1);
  try {
    traced_set c{1, 3, 5, 7, 9};
    FAIL();  // supposed to throw
  } catch (const std::bad_alloc&) {}
  EXPECT_EQ(default_alloc->allocs.size(), 3);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

TEST_F(countset_malloc_checks, no_leak_when_allocation_fails_mid_copy_ctor) {
  traced_set c{1, 3, 5, 7, 9};
  EXPECT_EQ(default_alloc->allocs.size(), 5);
  default_alloc->allocs.clear();
  default_alloc->end = default_alloc->end_capacity
      - 3 * traced_allocator<traced_set::node_type>::alloc_size(1);
  try {
    traced_set c2(c);  // NOLINT(performance-unnecessary-copy-initialization)
    FAIL();            // supposed to throw
  } catch (const std::bad_alloc&) {}
  EXPECT_EQ(default_alloc->allocs.size(), 3);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

struct maythrow_int {
  static inline int throws_after_n_changes = 0;

  int value;

  maythrow_int(): value{0} {
  }
  explicit maythrow_int(int val): value{val} {
  }
  maythrow_int(const maythrow_int& other): value{other.value} {
    throws_after_n_changes--;
    if (throws_after_n_changes <= 0) {
      throw std::runtime_error("maythrow_int");
    }
  }
  maythrow_int& operator=(const maythrow_int& other) {
    value = other.value;
    throws_after_n_changes--;
    if (throws_after_n_changes <= 0) {
      throw std::runtime_error("maythrow_int");
    }
    return *this;
  }
  ~maythrow_int() = default;

  [[nodiscard]] auto operator<=>(const maythrow_int& other) const noexcept
      = default;
};

using maythrow_countset
    = countset<maythrow_int, std::less<>, traced_allocator<maythrow_int>>;

TEST_F(countset_malloc_checks, no_leak_when_T_copy_fails_mid_ctor) {
  maythrow_int::throws_after_n_changes = 2;
  try {
    maythrow_countset c{maythrow_int(1), maythrow_int(3), maythrow_int(5)};
    FAIL();  // supposed to throw
  } catch (const std::runtime_error&) {}
  EXPECT_EQ(default_alloc->allocs.size(), 2);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}

TEST_F(countset_malloc_checks, no_leak_when_T_copy_fails_mid_copy_ctor) {
  maythrow_int::throws_after_n_changes = 5;
  maythrow_countset c{maythrow_int(1), maythrow_int(3), maythrow_int(5)};
  default_alloc->allocs.clear();
  try {
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    maythrow_countset c2(c);
    FAIL();  // supposed to throw
  } catch (const std::runtime_error&) {}
  EXPECT_EQ(default_alloc->allocs.size(), 2);
  EXPECT_EQ(default_alloc->allocs, default_alloc->deallocs);
}
