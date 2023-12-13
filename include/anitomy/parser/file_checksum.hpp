#pragma once

#include <algorithm>
#include <optional>
#include <ranges>
#include <vector>

#include "../element.hpp"
#include "../token.hpp"
#include "../util.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_file_checksum(std::vector<Token>& tokens_) noexcept {
  using namespace std::views;

  // A checksum has 8 hexadecimal digits (e.g. `ABCD1234`)
  static constexpr auto is_checksum = [](const Token& token) {
    return token.value.size() == 8 && std::ranges::all_of(token.value, is_xdigit);
  };

  // Find the last free token that is a checksum
  auto tokens = tokens_ | reverse | filter(is_free_token) | filter(is_checksum) | take(1);

  if (tokens.empty()) return std::nullopt;

  auto& token = tokens.front();

  token.element_kind = ElementKind::FileChecksum;

  return Element{
      .kind = ElementKind::FileChecksum,
      .value = token.value,
  };
}

}  // namespace anitomy::detail
