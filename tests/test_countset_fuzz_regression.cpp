#include "eeds/countset.h"
#include "test_countset_fuzzer.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

TEST(countset_regression, fuzz_regression_corpus) {
  const auto tester = make_countset_tester();

  const auto project_root_str = std::getenv("EEDS_PROJECT_ROOT");
  const auto project_root
      = (project_root_str != nullptr && project_root_str[0] != '\0')
      ? std::filesystem::path(project_root_str)
      : std::filesystem::current_path();
  const auto fuzz_corpus = project_root / "fuzz_corpus" / "countset";
  if (!std::filesystem::is_directory(fuzz_corpus)) {
    FAIL() << "Fuzz corpus not found.";
  }
  std::vector<std::byte> file_contents;
  for (const auto& path: std::filesystem::directory_iterator(fuzz_corpus)) {
    std::ifstream f(path, std::ios::binary);
    f.unsetf(std::ios::skipws);
    f.seekg(0, std::ios::end);
    auto size = f.tellg();
    file_contents.resize(static_cast<std::size_t>(size));
    f.seekg(0, std::ios::beg);
    f.read(reinterpret_cast<char*>(file_contents.data()), size);
    try {
      tester(file_contents.data(), file_contents.size());
    } catch (const std::exception& exc) {
      FAIL() << "Failed scenario " << path.path().stem().string() << " of size "
             << size << "B (path: " << std::filesystem::absolute(path)
             << "): " << exc.what();
    }
  }
}
