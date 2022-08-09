#include "countset_testing.h"
#include "eeds/countset.h"

#include <gtest/gtest.h>

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

TEST(countset, insert_copy) {
  eeds::countset<int> c{1, 3, 5};
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
  eeds::countset<int> c{1, 3, 5};
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
  eeds::countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  counted_ops::clear_counters();
  auto result = c.insert(counted_ops(4));
  ASSERT_EQ(counted_ops::copy_ctor_count, 0);
  ASSERT_EQ(counted_ops::copy_assign_count, 0);
  ASSERT_TRUE(result.second);
  ASSERT_EQ(*result.first, counted_ops(4));
  ASSERT_EQ(result.first, std::next(c.begin(), 2));
}

TEST(countset, insert_move_hint) {
  eeds::countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
  counted_ops::clear_counters();
  auto result = c.insert(std::next(c.begin(), 2), counted_ops(4));
  ASSERT_EQ(counted_ops::copy_ctor_count, 0);
  ASSERT_EQ(counted_ops::copy_assign_count, 0);
  ASSERT_EQ(*result, counted_ops(4));
  ASSERT_EQ(result, std::next(c.begin(), 2));
}

TEST(countset, insert_initializer_list) {
  eeds::countset<int> c{1, 3, 5, 7, 9};
  c.insert({2, 5, 4, 9, 10, 6});
  assert_countset_is(c, {1, 2, 3, 4, 5, 6, 7, 9, 10});
}

TEST(countset, insert_iterator_pair) {
  eeds::countset<int> c{1, 3, 5, 7, 9};
  std::vector<int> to_insert{2, 5, 4, 9, 10, 6};
  c.insert(to_insert.begin(), to_insert.end());
  assert_countset_is(c, {1, 2, 3, 4, 5, 6, 7, 9, 10});
}

TEST(countset, emplace) {
  eeds::countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
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
  eeds::countset<counted_ops> c{counted_ops(1), counted_ops(3), counted_ops(5)};
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
  eeds::countset<int> c{1, 3, 5, 7, 9};
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
  eeds::countset<int> c{1, 3, 5, 7, 9, 11, 13, 15};
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
  eeds::countset<int> c{1, 3, 5, 7, 9};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c1{1, 3, 5};
  c1.clear();
  assert_countset_is(c1, {});
  c1.insert({2, 4, 6});
  assert_countset_is(c1, {2, 4, 6});
  c1.clear();
  assert_countset_is(c1, {});
  c1.clear();
  assert_countset_is(c1, {});
}
