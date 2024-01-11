#pragma once

#include <algorithm>
#include <optional>
#include <span>

#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_episode_title(std::span<Token> tokens) noexcept {
  // Find the first free unenclosed range
  // e.g. `[Group] Title - Episode - Episode Title [Info]`
  //                                 ^-------------^
  auto token_begin = std::ranges::find_if(tokens, [](const Token& token) {
    return is_free_token(token) && !token.is_enclosed;  //
  });
  auto token_end = std::find_if(token_begin, tokens.end(), [](const Token& token) {
    return is_open_bracket_token(token) || token.element_kind.has_value();
  });

  auto span = std::span{token_begin, token_end};

  // Build episode title
  if (std::string value = build_element_value(span, KeepDelimiters::No); !value.empty()) {
    for (auto& token : span) {
      token.element_kind = ElementKind::EpisodeTitle;
    }
    return Element{
        .kind = ElementKind::EpisodeTitle,
        .value = value,
        .position = span.front().position,
    };
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
