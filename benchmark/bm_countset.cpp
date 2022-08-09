#include "benchmark/benchmark.h"
#include "eeds/countset.h"

#include <iostream>
#include <random>
#include <set>
#include <vector>

template<class Set>
using set_value_t = typename Set::value_type;

// Returns the set, a selection of existing values and a selection of
// non-existing values.
template<class Set>
std::tuple<Set, std::vector<set_value_t<Set>>, std::vector<set_value_t<Set>>>
    build_set(std::size_t size, auto alloc) {
  // It's deterministic on purpose.
  std::mt19937_64 rng;  // NOLINT(cert-msc51-cpp)
  std::uniform_int_distribution<set_value_t<Set>> distro;
  Set set(alloc);
  for (std::size_t i = 0; i < size; i++) {
    set.insert(distro(rng));
  }

  std::vector<set_value_t<Set>> values(set.begin(), set.end());
  // Try to select at random places in the set
  decltype(values) non_values{values.front() - 1, values.back() + 1};
  for (std::size_t i = 0; i + 1 < size; i++) {
    if (values[i] + 1 != values[i + 1]) {
      non_values.push_back(values[i] + 1);
    }
  }

  std::shuffle(values.begin(), values.end(), rng);
  if (values.size() > 64) {
    values = std::vector(values.begin(), values.begin() + 64);
  }
  // Always include min - 1 and max + 1, which are the first two.
  std::shuffle(non_values.begin() + 2, non_values.end(), rng);
  if (non_values.size() > 64) {
    non_values = std::vector(non_values.begin(), non_values.begin() + 64);
  }
  return {set, values, non_values};
}

template<class Set, class Op, bool hit_or_miss>
void bm_query(benchmark::State& state) {
  auto [s, v, nv] = build_set<Set>(static_cast<std::size_t>(state.range()),
                                   typename Set::allocator_type{});
  auto q = hit_or_miss ? v : nv;
  while (state.KeepRunning()) {
    for (auto val: q) {
      benchmark::DoNotOptimize(Op{}(s, val));
    }
  }
}

// An allocator that never deallocates. It only supports clearing all memory
// used. This is to decrease the allocation jitter and make the runtime of
// bm_mutator set-up and tear-down code bearable:
// After constructing the initial Set, the copies used for the benchmark can be
// pre-allocated (we know their total size).
struct clear_allocator_impl {
  std::size_t total_allocated = 0;
  std::size_t current_block_size;
  std::size_t current_block_cursor;
  std::vector<std::unique_ptr<std::byte[]>> blocks;

  clear_allocator_impl() {
    blocks.push_back(std::make_unique<std::byte[]>(4096));
    current_block_size = 4096;
    current_block_cursor = 0;
  }

  explicit clear_allocator_impl(std::size_t initial_block_size) {
    blocks.push_back(std::make_unique<std::byte[]>(initial_block_size));
    current_block_size = initial_block_size;
    current_block_cursor = 0;
  }

  void* allocate(std::size_t size) {
    total_allocated += size;
    if (current_block_cursor + size > current_block_size) {
      blocks.push_back(std::make_unique<std::byte[]>(4096));
      current_block_size = 4096;
      current_block_cursor = 0;
    }
    current_block_cursor += size;
    return blocks.back().get() + current_block_cursor - size;
  }
};

template<class T>
struct clear_allocator {
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

  clear_allocator_impl* i;

  clear_allocator() noexcept {
    std::cerr << "clear_allocator: default ctor" << std::endl;
    std::abort();
  }

  explicit clear_allocator(clear_allocator_impl* i) noexcept: i(i) {
  }

  template<class U>
  explicit clear_allocator(const clear_allocator<U>& a) noexcept: i(a.i) {
  }

  T* allocate(std::size_t n) {
    return reinterpret_cast<T*>(i->allocate(alloc_size(n)));
  }

  void deallocate(T*, std::size_t) noexcept {
  }

  bool operator==(const clear_allocator&) const noexcept = default;
};

template<class Set, class Op, bool hit_or_miss>
void bm_mutator(benchmark::State& state) {
  using Allocator = typename Set::allocator_type;

  clear_allocator_impl i;
  auto [s, v, nv]
      = build_set<Set>(static_cast<std::size_t>(state.range()), Allocator{&i});
  auto q = hit_or_miss ? v : nv;
  clear_allocator_impl i2(i.total_allocated);
  while (state.KeepRunning()) {
    {
      state.PauseTiming();
      Set s_copy(s, Allocator{&i2});
      state.ResumeTiming();
      for (auto val: q) {
        Op{}(s_copy, val);
      }
      state.PauseTiming();
      // s_copy dtor
    }
    i2.current_block_cursor = 0;
    state.ResumeTiming();
  }
}

#define DECLARE_BENCHMARKS(bm_func_template, name, op, allocator, ITERS)       \
  BENCHMARK(                                                                   \
      (bm_func_template<std::set<long, std::less<long>, allocator<long>>,      \
                        op,                                                    \
                        true>))                                                \
      ->RangeMultiplier(4)                                                     \
      ->Range(1, 1 << 18) ITERS->Name(name "/hit/std::set");                   \
  BENCHMARK(                                                                   \
      (bm_func_template<std::set<long, std::less<long>, allocator<long>>,      \
                        op,                                                    \
                        false>))                                               \
      ->RangeMultiplier(4)                                                     \
      ->Range(1, 1 << 18) ITERS->Name(name "/miss/std::set");                  \
  BENCHMARK(                                                                   \
      (bm_func_template<eeds::countset<long, std::less<>, allocator<long>>,    \
                        op,                                                    \
                        true>))                                                \
      ->RangeMultiplier(4)                                                     \
      ->Range(1, 1 << 18) ITERS->Name(name "/hit/countset");                   \
  BENCHMARK(                                                                   \
      (bm_func_template<eeds::countset<long, std::less<>, allocator<long>>,    \
                        op,                                                    \
                        false>))                                               \
      ->RangeMultiplier(4)                                                     \
      ->Range(1, 1 << 18) ITERS->Name(name "/miss/countset")

#define DECLARE_QUERY_BENCHMARKS(name, op)                                     \
  DECLARE_BENCHMARKS(bm_query, name, op, std::allocator, )

#define DECLARE_MUTATOR_BENCHMARKS(name, op)                                   \
  DECLARE_BENCHMARKS(bm_mutator, name, op, clear_allocator, ->Iterations(1000))

struct count_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.count(val);
  }
};
DECLARE_QUERY_BENCHMARKS("count", count_op);

struct contains_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.contains(val);
  }
};
DECLARE_QUERY_BENCHMARKS("contains", contains_op);

struct find_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.find(val);
  }
};
DECLARE_QUERY_BENCHMARKS("find", find_op);

struct lower_bound_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.lower_bound(val);
  }
};
DECLARE_QUERY_BENCHMARKS("lower_bound", lower_bound_op);

struct upper_bound_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.upper_bound(val);
  }
};
DECLARE_QUERY_BENCHMARKS("upper_bound", upper_bound_op);

struct equal_range_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.equal_range(val);
  }
};
DECLARE_QUERY_BENCHMARKS("equal_range", equal_range_op);

struct insert_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.insert(val);
  }
};
DECLARE_MUTATOR_BENCHMARKS("insert", insert_op);

struct erase_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.erase(val);
  }
};
DECLARE_MUTATOR_BENCHMARKS("erase", erase_op);
