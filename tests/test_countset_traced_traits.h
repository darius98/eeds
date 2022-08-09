#pragma once

#include "eeds/countset.h"

#include <gtest/gtest.h>
#include <set>

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
  eeds::default_rng r;
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

struct countset_traits : ::testing::Test {
  using traced_set = eeds::
      countset<int, traced_comparator<int>, traced_allocator<int>, traced_rng>;

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
