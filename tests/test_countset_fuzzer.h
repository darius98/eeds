#pragma once

#include "countset_testing.h"
#include "eeds/countset.h"

#include <cstdint>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>

enum class mutation_type : unsigned char {
  insert_single,
  insert_several,
  erase_value,
  erase_nth,
  erase_range,
};

template<bool print_ops, bool enable_checks>
struct countset_tester {
  static void run(const std::byte* begin, size_t size) {
    return countset_tester{}.push(begin, size);
  }

  void push(const std::byte* begin, size_t size) {
    const std::byte* end = begin + size;
    while (begin != end) {
      const auto op = static_cast<mutation_type>(*begin);
      begin += 1;
      switch (op) {
        case mutation_type::insert_single: insert_single(&begin, end); break;
        case mutation_type::insert_several: insert_several(&begin, end); break;
        case mutation_type::erase_value: erase_value(&begin, end); break;
        case mutation_type::erase_nth: erase_nth(&begin, end); break;
        case mutation_type::erase_range: erase_range(&begin, end); break;
      }
      if constexpr (enable_checks) {
        assert_countset_is(actual, expected);
      }
    }
  }

private:
  template<class T>
  static T safe_unaligned_load(const std::byte** begin, const std::byte* end) {
    T value{};
    if ((end - *begin) < static_cast<std::ptrdiff_t>(sizeof(T))) {
      *begin = end;
    } else {
      std::memcpy(&value, *begin, sizeof(T));
      *begin += sizeof(T);
    }
    return value;
  }

  void insert_single(const std::byte** begin, const std::byte* end) {
    const auto val = safe_unaligned_load<std::uint16_t>(begin, end);
    if constexpr (print_ops) {
      std::cerr << "INSERT_SINGLE   " << val << std::endl;
    }
    if constexpr (enable_checks) {
      auto it = std::lower_bound(expected.begin(), expected.end(), val);
      if (it == expected.end() || *it != val) {
        VERIFY(actual.insert(val).second);
        expected.insert(it, val);
      } else {
        VERIFY(!actual.insert(val).second);
      }
    } else {
      actual.insert(val);
    }
  }

  void insert_several(const std::byte** begin, const std::byte* end) {
    const auto cnt = safe_unaligned_load<std::uint8_t>(begin, end);
    std::vector<std::uint16_t> values;
    values.reserve(cnt);
    for (std::size_t i = 0; i < cnt && *begin != end; i++) {
      values.push_back(safe_unaligned_load<std::uint16_t>(begin, end));
    }
    if constexpr (print_ops) {
      std::cerr << "INSERT_SEVERAL  ";
      std::copy(values.begin(),
                values.end(),
                std::ostream_iterator<std::uint16_t>(std::cerr, " "));
      std::cerr << std::endl;
    }
    if constexpr (enable_checks) {
      expected.insert(expected.end(), values.begin(), values.end());
      std::sort(expected.begin(), expected.end());
      expected.erase(std::unique(expected.begin(), expected.end()),
                     expected.end());
    }
    actual.insert(values.begin(), values.end());
  }

  void erase_value(const std::byte** begin, const std::byte* end) {
    const auto val = safe_unaligned_load<std::uint16_t>(begin, end);
    if constexpr (print_ops) {
      std::cerr << "ERASE_SINGLE    " << val << std::endl;
    }
    if constexpr (enable_checks) {
      auto it = std::lower_bound(expected.begin(), expected.end(), val);
      if (it == expected.end() || *it != val) {
        VERIFY(actual.erase(val) == 0);
      } else {
        VERIFY(actual.erase(val) == 1);
        expected.erase(it);
      }
    } else {
      actual.erase(val);
    }
  }

  void erase_nth(const std::byte** begin, const std::byte* end) {
    if (actual.empty()) {
      return;
    }
    const auto n
        = safe_unaligned_load<std::uint16_t>(begin, end) % actual.size();
    if constexpr (print_ops) {
      std::cerr << "ERASE_NTH       " << n << std::endl;
    }
    if constexpr (enable_checks) {
      expected.erase(expected.begin() + static_cast<long>(n));
    }
    actual.erase(actual.nth(n));
  }

  void erase_range(const std::byte** begin, const std::byte* end) {
    if (actual.empty()) {
      return;
    }
    auto x = safe_unaligned_load<std::uint16_t>(begin, end) % actual.size();
    auto y = safe_unaligned_load<std::uint16_t>(begin, end) % actual.size();
    if (x > y) {
      std::swap(x, y);
    }
    if constexpr (print_ops) {
      std::cerr << "ERASE_NTH_RANGE " << x << " " << y << std::endl;
    }
    if constexpr (enable_checks) {
      expected.erase(expected.begin() + static_cast<long>(x),
                     expected.begin() + static_cast<long>(y + 1));
    }
    actual.erase(actual.nth(x), actual.nth(y + 1));
  }

  std::vector<std::uint16_t> expected{};
  eeds::countset<std::uint16_t> actual{};
};

inline auto make_countset_tester() -> void (*)(const std::byte*, std::size_t) {
  const auto print_ops_str = std::getenv("FUZZ_PRINT_OPS");
  const auto print_ops = (print_ops_str != nullptr && print_ops_str[0] != '\0'
                          && print_ops_str[0] != '0');
  const auto disable_checks_str = std::getenv("FUZZ_DISABLE_CHECKS");
  const auto enable_checks = disable_checks_str == nullptr
      || disable_checks_str[0] == '\0' || disable_checks_str[0] == '0';

  if (print_ops && enable_checks) {
    return countset_tester<true, true>::run;
  }
  if (print_ops) {
    return countset_tester<true, false>::run;
  }
  if (enable_checks) {
    return countset_tester<false, true>::run;
  }
  return countset_tester<false, false>::run;
}
