#pragma once

#include <optional>
#include <ranges>
#include <span>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_anime_title(std::span<Token> tokens) noexcept {
  // Find the first free unenclosed token
  auto token_begin = std::ranges::find_if(
      tokens, [](const Token& token) { return is_free_token(token) && !token.is_enclosed; });

  // If that doesn't work, find the first unknown token in the second enclosed group (assuming
  // that the first one is the release group)
  if (token_begin == tokens.end()) {
    // @TODO
    return std::nullopt;
  }

  // Continue until an identifier is found
  auto token_end = std::ranges::find_if(
      token_begin, tokens.end(), [](const Token& token) { return token.element_kind.has_value(); });

  // If the interval ends with an enclosed group (e.g. "Anime Title [Fansub]"), move the upper
  // endpoint back to the beginning of the group. We ignore parentheses in order to keep certain
  // groups (e.g. "(TV)") intact.
  //
  // @TODO

  // Trim delimiters and open brackets
  static constexpr auto is_invalid_token = [](const Token& token) {
    return token.kind == TokenKind::Delimiter || token.kind == TokenKind::OpenBracket;
  };
  while (token_end != tokens.begin() && token_end != tokens.end() &&
         is_invalid_token(*std::prev(token_end))) {
    token_end = std::prev(token_end);
  }

  auto span = std::span(token_begin, token_end);

  // Build anime title
  if (std::string value = build_element_value(span); !value.empty()) {
    for (auto& token : span) {
      token.element_kind = ElementKind::AnimeTitle;
    }
    return Element{
        .kind = ElementKind::AnimeTitle,
        .value = value,
    };
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
