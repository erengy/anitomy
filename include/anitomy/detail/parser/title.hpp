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

inline std::span<Token> find_title(std::span<Token> tokens) noexcept {
  // Find the first free unenclosed range
  // e.g. `[Group] Title - Episode [Info]`
  //               ^-------^
  auto first = std::ranges::find_if(tokens, [](const Token& token) {
    return is_free_token(token) && !token.is_enclosed;  //
  });
  auto last = std::find_if(first, tokens.end(), is_identified_token);

  // Fall back to the second enclosed range (assuming the first one is for release group)
  // e.g. `[Group][Title][Info]`
  //               ^----^
  if (first == tokens.end()) {
    first = std::ranges::find_if(tokens, is_close_bracket_token);
    first = std::find_if(first, tokens.end(), is_free_token);
    last = std::find_if(first, tokens.end(), is_bracket_token);
  }

  // Allow filenames without a title
  if (first == tokens.end()) return {};

  // Prevent titles with mismatched brackets
  // e.g. `Title (`      -> `Title `
  // e.g. `Title [Info ` -> `Title `
  if (const auto open_brackets = find_all_if(first, last, is_open_bracket_token);
      !open_brackets.empty()) {
    if (std::ranges::count_if(first, last, is_close_bracket_token) != open_brackets.size()) {
      last = open_brackets.back();
    }
  }

  // Prevent titles ending with brackets (except parentheses)
  // e.g. `Title [Group]` -> `Title `
  // e.g. `Title (TV)`    -> *no change*
  if (auto token = find_prev_token(tokens, last, is_not_delimiter_token);
      is_close_bracket_token(*token) && token->value != ")") {
    if (token = find_prev_token(tokens, token, is_open_bracket_token); token != tokens.end()) {
      last = token;
    }
  }

  return {first, last};
}

inline std::optional<Element> parse_title(std::span<Token> tokens) noexcept {
  const auto span = find_title(tokens);
  if (span.empty()) return {};

  std::string value = build_element_value(span, KeepDelimiters::No);
  if (value.empty()) return {};

  for (auto& token : span) {
    token.element_kind = ElementKind::Title;
  }

  return Element{
      .kind = ElementKind::Title,
      .value = std::move(value),
      .position = span.front().position,
  };
}

}  // namespace anitomy::detail
