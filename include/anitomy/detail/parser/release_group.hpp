#pragma once

#include <algorithm>
#include <optional>
#include <span>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_release_group(std::span<Token> tokens) noexcept {
  // Find the first free enclosed range
  // e.g. `[Group] Title - Episode [Info]`
  //        ^----^
  auto token_begin = std::ranges::find_if(tokens, [](const Token& token) {
    return is_free_token(token) && token.is_enclosed;  //
  });
  auto token_end = std::find_if(token_begin, tokens.end(), [](const Token& token) {
    return is_close_bracket_token(token) || token.element_kind.has_value();
  });

  // Skip if the range contains other tokens
  if (auto token = find_prev_token(tokens, token_begin, is_not_delimiter_token);
      token != tokens.end() && !is_open_bracket_token(*token)) {
    return parse_release_group({token_end, tokens.end()});
  }
  if (token_end != tokens.end() && !is_close_bracket_token(*token_end)) {
    return parse_release_group({token_end, tokens.end()});
  }

  auto span = std::span{token_begin, token_end};

  // Build release group
  if (std::string value = build_element_value(span, false); !value.empty()) {
    for (auto& token : span) {
      token.element_kind = ElementKind::ReleaseGroup;
    }
    return Element{
        .kind = ElementKind::ReleaseGroup,
        .value = value,
        .position = span.front().position,
    };
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
