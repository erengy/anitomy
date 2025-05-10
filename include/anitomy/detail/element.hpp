#pragma once

#include <algorithm>
#include <ranges>
#include <set>
#include <span>
#include <string>

#include <anitomy/detail/delimiter.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/detail/unicode.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

enum class KeepDelimiters { No, Yes };

inline Element element_from_token(ElementKind kind, const Token& token, std::string_view value = {},
                                  size_t position = std::string::npos) {
  return Element{
      .kind = kind,
      .value = value.empty() ? token.value : std::string{value},
      .position = position != std::string::npos ? position : token.position,
  };
};

inline std::string build_element_value(std::span<Token> tokens,
                                       const KeepDelimiters keep_delimiters) noexcept {
  static constexpr auto first_code_point = [](const Token& token) {
    return unicode::utf8::decode(token.value).code_point;
  };

  const auto delimiters = tokens | std::views::filter(is_delimiter_token) |
                          std::views::transform(first_code_point) |
                          std::ranges::to<std::set<char32_t>>();

  const bool has_single_delimiter = delimiters.size() == 1;
  const bool has_spaces = std::ranges::any_of(delimiters, is_space);
  const bool has_underscores = delimiters.contains(U'_');

  const auto is_transformable_delimiter = [&](const Token& token) {
    if (keep_delimiters == KeepDelimiters::Yes) return false;
    if (is_not_delimiter_token(token)) return false;

    const char32_t ch = first_code_point(token);

    if (ch == ',' || ch == '&' || ch == '~') return false;  // keep
    if (is_space(ch) || ch == '_') return true;             // transform
    if (has_spaces || has_underscores) return false;        // keep
    if (ch == '.') return true;                             // transform
    return has_single_delimiter;                            // transform
  };

  if (keep_delimiters == KeepDelimiters::No) {
    unicode::code_point_t prev_delimiter{};
    while (!tokens.empty() && is_delimiter_token(tokens.back())) {
      const auto delimiter = first_code_point(tokens.back());
      if (delimiter == '~') break;
      if (delimiter == '.' && is_space(prev_delimiter)) break;
      prev_delimiter = delimiter;
      tokens = tokens.first(tokens.size() - 1);  // trim
    }
  }

  std::string element_value;

  for (const auto& token : tokens) {
    if (is_transformable_delimiter(token)) {
      element_value.push_back(' ');
    } else {
      element_value.append(token.value);
    }
  }

  return element_value;
}

}  // namespace anitomy::detail
