#pragma once

#include <algorithm>
#include <optional>
#include <span>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::span<Token> find_release_group(std::span<Token> tokens) noexcept {
  // Find the first enclosed unidentified range
  // e.g. `[Group] Title - Episode [Info]`
  //        ^----^
  auto first = std::ranges::find_if(tokens, [](const Token& token) {
    return token.is_enclosed && !is_identified_token(token);  //
  });
  auto last = std::find_if(first, tokens.end(), [](const Token& token) {
    return is_close_bracket_token(token) || is_identified_token(token);
  });

  // Skip if the range contains other tokens
  if (first != tokens.end()) {
    if (auto token = find_prev_token(tokens, first, is_not_delimiter_token);
        token != tokens.end() && !is_open_bracket_token(*token)) {
      return find_release_group({last, tokens.end()});
    }
    if (last != tokens.end() && !is_close_bracket_token(*last)) {
      return find_release_group({last, tokens.end()});
    }
  }

  // Fall back to the last token before file extension
  // e.g. `Title.Episode.Info-Group.mkv`
  //                          ^----^
  if (first == tokens.end()) {
    auto token = find_prev_token(tokens, tokens.end(), [](const Token& token) {
      return token.element_kind != ElementKind::FileExtension && is_not_delimiter_token(token);
    });
    if (token != tokens.end() && is_free_token(*token)) {
      auto prev_token = find_prev_token(tokens, token, [](const Token&) { return true; });
      if (prev_token != tokens.end() && is_delimiter_token(*prev_token) &&
          prev_token->value == "-") {
        first = token;
        last = std::next(token);
      }
    }
  }

  return {first, last};
}

inline std::optional<Element> parse_release_group(std::span<Token> tokens) noexcept {
  const auto span = find_release_group(tokens);
  if (span.empty()) return {};

  std::string value = build_element_value(span, KeepDelimiters::Yes);
  if (value.empty()) return {};

  for (auto& token : span) {
    token.element_kind = ElementKind::ReleaseGroup;
  }

  return Element{
      .kind = ElementKind::ReleaseGroup,
      .value = std::move(value),
      .position = span.front().position,
  };
}

}  // namespace anitomy::detail
