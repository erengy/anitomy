#pragma once

#include <ranges>
#include <set>
#include <span>
#include <string>

#include <anitomy/detail/token.hpp>

namespace anitomy::detail {

enum class KeepDelimiters { No, Yes };

inline std::string build_element_value(std::span<Token> tokens,
                                       const KeepDelimiters keep_delimiters) noexcept {
  const std::set<char> delimiters =
      tokens | std::views::filter(is_delimiter_token) |
      std::views::transform([](const Token& token) { return token.value.front(); }) |
      std::ranges::to<std::set<char>>();

  const bool has_multiple_delimiters = delimiters.size() > 1;

  const auto is_transformable = [&](const Token& token) {
    if (is_not_delimiter_token(token) || keep_delimiters == KeepDelimiters::Yes) {
      return false;
    }
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

  if (keep_delimiters == KeepDelimiters::No) {
    while (!tokens.empty() && is_delimiter_token(tokens.back())) {
      tokens = tokens.first(tokens.size() - 1);  // trim
    }
  }

  std::string element_value;

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
