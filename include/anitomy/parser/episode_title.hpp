#pragma once

#include <algorithm>
#include <optional>
#include <vector>

#include "../container.hpp"
#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

// @TODO: https://github.com/erengy/anitomy/issues/13
inline std::optional<Element> parse_episode_title(std::vector<Token>& tokens_) noexcept {
  auto token_begin = tokens_.begin();
  auto token_end = tokens_.begin();

  do {
    // Find the first free unenclosed token
    token_begin = std::ranges::find_if(token_end, tokens_.end(), [](const Token& token) {
      return is_free_token(token) && !token.is_enclosed;
    });
    if (token_begin == tokens_.end()) break;

    // Continue until a bracket or identifier is found
    token_end = std::ranges::find_if(token_begin, tokens_.end(), [](const Token& token) {
      return token.kind == TokenKind::OpenBracket || token.element_kind.has_value();
    });

    // @TODO: Ignore if it's only a dash

    auto span = std::span(token_begin, token_end);

    // Trim delimiters
    while (!span.empty() && span.back().kind == TokenKind::Delimiter) {
      span = span.first(span.size() - 1);
    }

    // Build episode title
    if (std::string value = build_element_value(span); !value.empty()) {
      for (auto& token : span) {
        token.element_kind = ElementKind::EpisodeTitle;
      }
      return Element{
          .kind = ElementKind::EpisodeTitle,
          .value = value,
      };
    }
    break;
  } while (token_begin != tokens_.end());

  return std::nullopt;
}

}  // namespace anitomy::detail
