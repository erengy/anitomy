#pragma once

#include <algorithm>
#include <optional>
#include <span>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_title(std::span<Token> tokens) noexcept {
  // Find the first free unenclosed range
  // e.g. `[Group] Title - Episode [Info]`
  //               ^-------^
  auto token_begin = std::ranges::find_if(tokens, [](const Token& token) {
    return is_free_token(token) && !token.is_enclosed;  //
  });
  auto token_end = std::find_if(token_begin, tokens.end(), [](const Token& token) {
    return token.element_kind.has_value();  //
  });

  // Fall back to the second enclosed range (assuming the first one is for release group)
  // e.g. `[Group][Title][Info]`
  //               ^----^
  if (token_begin == tokens.end()) {
    token_begin = std::ranges::find_if(tokens, is_close_bracket_token);
    token_begin = std::find_if(token_begin, tokens.end(), is_free_token);
    token_end = std::find_if(token_begin, tokens.end(), is_bracket_token);
  }

  // Allow filenames without a title
  if (token_begin == tokens.end()) return std::nullopt;

  // Prevent titles with mismatched brackets
  // e.g. `Title (`      -> `Title `
  // e.g. `Title [Info ` -> `Title `
  if (const auto open_brackets = find_all_if(token_begin, token_end, is_open_bracket_token);
      !open_brackets.empty()) {
    if (std::ranges::count_if(token_begin, token_end, is_close_bracket_token) !=
        open_brackets.size()) {
      token_end = open_brackets.back();
    }
  }

  // Prevent titles ending with brackets (except parentheses)
  // e.g. `Title [Group]` -> `Title `
  // e.g. `Title (TV)`    -> *no change*
  if (auto token = find_prev_token(tokens, token_end, is_not_delimiter_token);
      is_close_bracket_token(*token) && token->value != ")") {
    if (token = find_prev_token(tokens, token, is_open_bracket_token); token != tokens.end()) {
      token_end = token;
    }
  }

  auto span = std::span{token_begin, token_end};

  // Trim delimiters
  while (!span.empty() && is_delimiter_token(span.back())) {
    span = span.first(span.size() - 1);
  }

  // Build the title
  if (std::string value = build_element_value(span); !value.empty()) {
    for (auto& token : span) {
      token.element_kind = ElementKind::Title;
    }
    return Element{
        .kind = ElementKind::Title,
        .value = value,
        .position = span.front().position,
    };
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
