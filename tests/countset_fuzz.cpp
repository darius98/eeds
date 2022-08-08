#include "countset_test.h"
#include "eeds/countset.h"

static const auto tester = make_countset_tester();

extern "C" [[maybe_unused]] int LLVMFuzzerTestOneInput(const std::byte* begin,
                                                       std::size_t size) {
  tester(begin, size);
  return 0;
}
