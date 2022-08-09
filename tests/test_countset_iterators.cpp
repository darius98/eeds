#include "countset_testing.h"
#include "eeds/countset.h"

#include <gtest/gtest.h>
#include <iterator>

TEST(countset, iterator_types) {
  eeds::countset<int> c{};

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
  eeds::countset<int> c{1};
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
  eeds::countset<int> c{1};
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
  eeds::countset<int> c{1, 3, 5};

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
  eeds::countset<int> c{1, 3, 5};

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
  eeds::countset<std::pair<int, int>, pair_cmp> c{{1, 2}, {3, 4}, {5, 6}};

  auto it = c.begin();
  ASSERT_EQ(*it, (std::pair{1, 2}));
  ASSERT_EQ(it->first, 1);
  ASSERT_EQ(it->second, 2);
  it++;
  ASSERT_EQ(*it, (std::pair{3, 4}));
  ASSERT_EQ(it->first, 3);
  ASSERT_EQ(it->second, 4);
  it++;
  ASSERT_EQ(*it, (std::pair{5, 6}));
  ASSERT_EQ(it->first, 5);
  ASSERT_EQ(it->second, 6);
}
