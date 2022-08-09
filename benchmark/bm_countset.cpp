#include "benchmark/benchmark.h"
#include "eeds/countset.h"

#include <random>
#include <set>
#include <vector>

template<class Set>
using set_value_t = typename Set::value_type;

static void args(benchmark::internal::Benchmark* bm) {
  bm->Arg(1 << 0)
      ->Arg(1 << 2)
      ->Arg(1 << 4)
      ->Arg(1 << 6)
      ->Arg(1 << 8)
      ->Arg(1 << 10)
      ->Arg(1 << 12)
      ->Arg(1 << 14)
      ->Arg(1 << 16)
      ->Arg(1 << 18);
}

// Returns the set, a selection of existing values and a selection of
// non-existing values.
template<class Set>
std::tuple<Set, std::vector<set_value_t<Set>>, std::vector<set_value_t<Set>>>
    build_set(std::size_t size) {
  // It's deterministic on purpose.
  std::mt19937_64 rng;  // NOLINT(cert-msc51-cpp)
  std::uniform_int_distribution<set_value_t<Set>> distro;
  Set set;
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
void bm_getter(benchmark::State& state) {
  auto [s, v, nv] = build_set<Set>(static_cast<std::size_t>(state.range()));
  auto q = hit_or_miss ? v : nv;
  while (state.KeepRunning()) {
    for (auto val: q) {
      benchmark::DoNotOptimize(Op{}(s, val));
    }
  }
}

#define DECLARE_BENCHMARKS(name, op)                                           \
  BENCHMARK(bm_getter<std::set<long>, op, false>)                              \
      ->Apply(args)                                                            \
      ->Name(name "/miss/std::set");                                           \
  BENCHMARK(bm_getter<std::set<long>, op, true>)                               \
      ->Apply(args)                                                            \
      ->Name(name "/hit/std::set");                                            \
  BENCHMARK(bm_getter<eeds::countset<long>, op, false>)                        \
      ->Apply(args)                                                            \
      ->Name(name "/miss/countset");                                           \
  BENCHMARK(bm_getter<eeds::countset<long>, op, true>)                         \
      ->Apply(args)                                                            \
      ->Name(name "/hit/countset")

struct count_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.count(val);
  }
};
DECLARE_BENCHMARKS("count", count_op);

struct contains_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.contains(val);
  }
};
DECLARE_BENCHMARKS("contains", contains_op);

struct find_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.find(val);
  }
};
DECLARE_BENCHMARKS("find", find_op);

struct lower_bound_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.lower_bound(val);
  }
};
DECLARE_BENCHMARKS("lower_bound", lower_bound_op);

struct upper_bound_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.upper_bound(val);
  }
};
DECLARE_BENCHMARKS("upper_bound", upper_bound_op);

struct equal_range_op {
  [[gnu::always_inline, gnu::hot]] auto operator()(auto&& set, auto&& val) {
    return set.equal_range(val);
  }
};
DECLARE_BENCHMARKS("equal_range", equal_range_op);
