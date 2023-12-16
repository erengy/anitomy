#pragma once

#include <algorithm>
#include <print>
#include <ranges>
#include <string>
#include <vector>

namespace anitomy::detail {

inline void print_table(std::vector<std::string> headers,
                        std::vector<std::vector<std::string>> rows) noexcept {
  auto column_widths = headers |
                       std::views::transform([](const std::string& s) { return s.size(); }) |
                       std::ranges::to<std::vector>();
  for (const auto& columns : rows) {
    for (size_t i = 0; i < columns.size(); ++i) {
      column_widths[i] = std::max(column_widths[i], columns[i].size());
    }
  }

  const size_t table_width =
      std::ranges::fold_left(column_widths, 0, std::plus<>{}) + (column_widths.size() * 3) - 1;

  const auto print_row = [&](const std::vector<std::string>& columns) {
    for (size_t i = 0; i < columns.size(); ++i) {
      std::print("│ {:<{}} ", columns[i], column_widths[i]);
    }
    std::print("│\n");
  };

  std::println("┌{:─^{}}┐", "", table_width);
  print_row(headers);
  std::println("│{:─^{}}│", "", table_width);
  std::ranges::for_each(rows, print_row);
  std::println("└{:─^{}}┘", "", table_width);
}

}  // namespace anitomy::detail
