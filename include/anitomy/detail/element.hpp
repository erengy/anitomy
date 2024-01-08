#pragma once

#include <ranges>
#include <set>
#include <span>
#include <string>

#include <anitomy/detail/token.hpp>

namespace anitomy::detail {

inline std::string build_element_value(const std::span<Token> tokens,
                                       const bool transform_delimiters = true) noexcept {
  std::string element_value;

  const bool has_multiple_delimiters = [&tokens]() {
    constexpr auto token_value = [](const Token& token) { return token.value.front(); };
    auto delimiters_view =
        tokens | std::views::filter(is_delimiter_token) | std::views::transform(token_value);
    std::set<char> delimiters{delimiters_view.begin(), delimiters_view.end()};
    return delimiters.size() > 1;
  }();

  const auto is_transformable = [&](const Token& token) {
    if (token.kind != TokenKind::Delimiter || !transform_delimiters) return false;
    switch (token.value.front()) {
      case ',':
      case '&':
        return false;
      case '_':
        return true;
      default:
        return !has_multiple_delimiters;
    }
  };

  for (const auto& token : tokens) {
    if (is_transformable(token)) {
      element_value.push_back(' ');
    } else {
      element_value.append(token.value);
    }
  }

  return element_value;
}

}  // namespace anitomy::detail
