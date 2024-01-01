#pragma once

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>

#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_file_checksum(std::span<Token> tokens) noexcept {
  using namespace std::views;

  // A checksum has 8 hexadecimal digits (e.g. `ABCD1234`)
  static constexpr auto is_checksum = [](const Token& token) {
    return token.value.size() == 8 && std::ranges::all_of(token.value, is_xdigit);
  };

  // Find the last free token that is a checksum
  auto view = tokens | reverse | filter(is_free_token) | filter(is_checksum) | take(1);

  if (view.empty()) return std::nullopt;

  auto& token = view.front();

  token.element_kind = ElementKind::FileChecksum;

  return Element{
      .kind = ElementKind::FileChecksum,
      .value = token.value,
      .position = token.position,
  };
}

}  // namespace anitomy::detail
