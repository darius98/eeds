#include "countset_testing.h"
#include "eeds/countset.h"
#include "test_countset_traced_traits.h"

#include <gtest/gtest.h>
#include <vector>

// Ensure all functions are instantiated, so we have accurate coverage
// information.
template struct eeds::
    countset<int, traced_comparator<int>, traced_allocator<int>, traced_rng>;

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
