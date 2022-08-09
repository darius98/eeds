#include "countset_testing.h"
#include "eeds/countset.h"

#include <gtest/gtest.h>

TEST(countset, count) {
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c{};
  ASSERT_EQ(c.find(0), c.end());
}

TEST(countset, find) {
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{};
  ASSERT_EQ(c.find(0), c.end());
}

TEST(countset, find_transparent) {
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c{};
  ASSERT_EQ(c.lower_bound(0), c.end());
}

TEST(countset, lower_bound) {
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{};
  ASSERT_EQ(c.lower_bound(0), c.end());
}

TEST(countset, lower_bound_transparent) {
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c{};
  ASSERT_EQ(c.upper_bound(0), c.end());
}

TEST(countset, upper_bound) {
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{};
  ASSERT_EQ(c.upper_bound(0), c.end());
}

TEST(countset, upper_bound_transparent) {
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
  eeds::countset<int> c{};
  ASSERT_EQ(c.equal_range(0), std::pair(c.end(), c.end()));
}

TEST(countset, equal_range) {
  eeds::countset<int> c{1, 3, 5, 7};
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
  eeds::countset<std::pair<int, int>, pair_cmp> c{};
  ASSERT_EQ(c.equal_range(0), std::pair(c.end(), c.end()));
}

TEST(countset, equal_range_transparent) {
  eeds::countset<std::pair<int, int>, pair_cmp> c{
      {1, 2}, {3, 4}, {3, 6}, {3, 8}, {5, 4}, {7, 6}};
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
