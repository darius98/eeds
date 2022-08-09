#pragma once

#include "eeds/countset.h"

#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

#define INTERNAL_CHECK_STRINGIFY2(line) #line
#define INTERNAL_CHECK_STRINGIFY(line) INTERNAL_CHECK_STRINGIFY2(line)
#define VERIFY(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      throw std::runtime_error("ASSERTION FAILED: " INTERNAL_CHECK_STRINGIFY(  \
          expr) " at " __FILE__ ":" INTERNAL_CHECK_STRINGIFY(__LINE__));       \
    }                                                                          \
  } while (false)

template<class T, class C, class A, class R>
void test_countset_sanity(const eeds::countset<T, C, A, R>& actual) {
  std::unordered_set<const eeds::countset_node<T>*> visited;
  const C cmp = actual.key_comp();
  std::queue<const eeds::countset_node<T>*> nodes_queue;
  const auto root = actual.get_root_node();
  if (root == nullptr) {
    VERIFY(actual.empty());
    VERIFY(actual.size() == 0);
    return;
  }
  nodes_queue.push(root);
  while (!nodes_queue.empty()) {
    const auto node = nodes_queue.front();
    nodes_queue.pop();
    if (node == nullptr) {
      continue;
    }
    VERIFY(!visited.contains(node));  // It's a cyclic "tree"!
    visited.insert(node);
    std::size_t expected_count = 1;
    if (node->get_left() != nullptr) {
      VERIFY(node->get_priority() >= node->get_left()->get_priority());
      VERIFY(!cmp(node->value(), node->get_left()->value()));
      VERIFY(node->get_left()->get_parent() == node);
      expected_count += node->get_left()->get_count();
      nodes_queue.push(node->get_left());
    }
    if (node->get_right() != nullptr) {
      VERIFY(node->get_priority() >= node->get_right()->get_priority());
      VERIFY(!cmp(node->get_right()->value(), node->value()));
      VERIFY(node->get_right()->get_parent() == node);
      expected_count += node->get_right()->get_count();
      nodes_queue.push(node->get_right());
    }
    VERIFY(expected_count == node->get_count());
  }
  VERIFY(actual.get_root_node()->get_parent() == nullptr);
}

template<class T, class C, class A, class R>
void assert_countset_is(const eeds::countset<T, C, A, R>& actual,
                        const std::vector<T>& expected) {
  test_countset_sanity(actual);
  VERIFY(expected.size() == actual.size());
  VERIFY(expected.empty() == actual.empty());
  std::size_t index = 0;
  for (auto it = actual.begin(); it != actual.end(); it++) {
    auto v = expected[index];
    VERIFY(v == *it);
    VERIFY(index == it.index());
    VERIFY(actual.count(v) == 1);
    VERIFY(actual.contains(v));
    VERIFY(actual.find(v) == it);
    VERIFY(actual.lower_bound(v) == it);
    VERIFY(actual.upper_bound(v) == std::next(it));
    VERIFY(actual.equal_range(v) == std::pair(it, std::next(it)));
    VERIFY(actual.nth(index) == it);
    if constexpr (std::is_integral_v<T>) {
      if (index != 0 && expected[index - 1] + 1 < v) {
        auto nv = expected[index - 1] + 1;
        VERIFY(actual.count(nv) == 0);
        VERIFY(!actual.contains(nv));
        VERIFY(actual.find(nv) == actual.end());
        VERIFY(actual.lower_bound(nv) == it);
        VERIFY(actual.upper_bound(nv) == it);
        VERIFY(actual.equal_range(nv) == std::pair(it, it));
      }
    }
    index++;
  }
  VERIFY(actual.nth(index) == actual.end());
}

// A transparent comparison of just the first element of a pair.
struct pair_cmp {
  using is_transparent [[maybe_unused]] = void;

  template<class T, class U>
  [[nodiscard]] auto operator()(const std::pair<T, U>& a,
                                const std::pair<T, U>& b) const noexcept {
    return a < b;
  }

  template<class T, class U>
  [[nodiscard]] auto operator()(const T& a,
                                const std::pair<T, U>& b) const noexcept {
    return a < b.first;
  }
  template<class T, class U>
  [[nodiscard]] auto operator()(const std::pair<T, U>& a,
                                const T& b) const noexcept {
    return a.first < b;
  }
};
