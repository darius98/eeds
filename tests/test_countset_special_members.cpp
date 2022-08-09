#include "countset_testing.h"
#include "eeds/countset.h"

#include <gtest/gtest.h>
#include <list>
#include <vector>

TEST(countset, default_ctor) {
  eeds::countset<int> c;
  assert_countset_is(c, {});

  // Make sure it's implicit
  using T = int;
  using C = std::less<>;
  using A = std::allocator<int>;
  using R = eeds::default_rng;
  assert_countset_is<T, C, A, R>({}, {});
}

TEST(countset, init_list_ctor) {
  eeds::countset<int> c{1, 3, 5};
  assert_countset_is(c, {1, 3, 5});

  // Make sure it's implicit
  using T = int;
  using C = std::less<>;
  using A = std::allocator<int>;
  using R = eeds::default_rng;
  assert_countset_is<T, C, A, R>({1, 3, 5}, {1, 3, 5});
}

TEST(countset, iterator_pair_ctor) {
  {
    // random access
    std::vector<int> v{1, 3, 5};
    eeds::countset<int> c(v.begin(), v.end());
    assert_countset_is(c, {1, 3, 5});
  }
  {
    // bidirectional
    std::list<int> v{1, 3, 5};
    eeds::countset<int> c(v.begin(), v.end());
    assert_countset_is(c, {1, 3, 5});
  }
  {
    // input
    std::stringstream sin{"1 3 5"};
    eeds::countset<int> c(std::istream_iterator<int>{sin},
                          std::istream_iterator<int>{});
    assert_countset_is(c, {1, 3, 5});
  }
}

TEST(countset, copy_ctor) {
  auto* c = new eeds::countset<int>{1, 3, 5};
  eeds::countset<int> c2(
      *c);  // NOLINT(performance-unnecessary-copy-initialization)
  delete c;
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, copy_ctor_empty) {
  auto* c = new eeds::countset<int>{};
  eeds::countset<int> c2(
      *c);  // NOLINT(performance-unnecessary-copy-initialization)
  delete c;
  assert_countset_is(c2, {});
}

TEST(countset, move_ctor) {
  auto* c = new eeds::countset<int>{1, 3, 5};
  eeds::countset<int> c2(std::move(*c));
  delete c;
  assert_countset_is(c2, {1, 3, 5});
}

TEST(countset, move_ctor_empty) {
  auto* c = new eeds::countset<int>{};
  eeds::countset<int> c2(std::move(*c));
  delete c;
  assert_countset_is(c2, {});
}

TEST(countset, copy_assign) {
  eeds::countset<int> c2;
  auto* c = new eeds::countset<int>{1, 3, 5};
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
  eeds::countset<int> c2;
  auto* c = new eeds::countset<int>{1, 3, 5};
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
  eeds::countset<int> c1{1, 3, 5};
  eeds::countset<int> c2{2, 4, 6};
  c1.swap(c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  c1.swap(c1);
  assert_countset_is(c1, {2, 4, 6});
}

TEST(countset, swap_hidden_friend) {
  eeds::countset<int> c1{1, 3, 5};
  eeds::countset<int> c2{2, 4, 6};
  swap(c1, c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  swap(c1, c1);
  assert_countset_is(c1, {2, 4, 6});
}

TEST(countset, std_swap) {
  eeds::countset<int> c1{1, 3, 5};
  eeds::countset<int> c2{2, 4, 6};
  // Note: This is less efficient than the hidden friend implementation, prefer
  // using the insane pattern of "using std::swap; swap(a, b);".
  std::swap(c1, c2);
  assert_countset_is(c1, {2, 4, 6});
  assert_countset_is(c2, {1, 3, 5});

  // Make sure self-swap works
  std::swap(c1, c1);
  assert_countset_is(c1, {2, 4, 6});
}
