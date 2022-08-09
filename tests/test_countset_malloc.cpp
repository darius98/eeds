#include "eeds/countset.h"
#include "test_countset_traced_traits.h"

#include <gtest/gtest.h>

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
    = eeds::countset<maythrow_int, std::less<>, traced_allocator<maythrow_int>>;

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
