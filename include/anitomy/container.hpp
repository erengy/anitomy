#pragma once

#include <algorithm>
#include <functional>
#include <set>
#include <span>
#include <string>
#include <vector>

#include "token.hpp"

namespace anitomy::detail {

using token_container_t = std::vector<Token>;
using token_iterator_t = token_container_t::iterator;
using token_predicate_t = std::function<bool(const Token&)>;

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

inline token_iterator_t find_prev_token(token_container_t& container, token_iterator_t it,
                                        token_predicate_t predicate) noexcept {
  auto [token, end] = std::ranges::find_last_if(container.begin(), it, predicate);
  return token;
}

inline token_iterator_t find_next_token(token_container_t& container, token_iterator_t it,
                                        token_predicate_t predicate) noexcept {
  if (it == container.end()) return container.end();
  return std::ranges::find_if(std::next(it), container.end(), predicate);
}

}  // namespace anitomy::detail
