#pragma once

#include <algorithm>
#include <optional>
#include <span>

#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::span<Token> find_episode_title(std::span<Token> tokens) noexcept {
  static constexpr auto is_invalid_token = [](const Token& token) {
    if (!is_identified_token(token)) return false;
    return token.element_kind != ElementKind::Episode &&
           token.element_kind != ElementKind::ReleaseVersion &&
           token.element_kind != ElementKind::Season;
  };

  static constexpr auto is_part_token = [](const Token& token) {
    return token.element_kind == ElementKind::Part;
  };

  const auto episode = std::ranges::find_if(tokens, [](const Token& token) {
    return token.element_kind == ElementKind::Episode;  //
  });

  if (episode == tokens.end()) return {};

  // Find the first free unenclosed range after episode
  // e.g. `[Group] Title - Episode - Episode Title [Info]`
  //                                 ^-------------^
  auto first = std::ranges::find_if(episode, tokens.end(), [](const Token& token) {
    return is_free_token(token) && !is_enclosed_token(token);  //
  });
  if (std::ranges::any_of(episode, first, is_invalid_token)) {
    first = tokens.end();
  }
  auto last = std::find_if(first, tokens.end(), [](const Token& token) {
    return is_open_bracket_token(token) || (is_identified_token(token) && !is_part_token(token));
  });

  // Fall back to the first free range in corner brackets after episode
  // e.g. `[Group] Title - Episode 「Episode Title」`
  //                                ^------------^
  if (first == tokens.end()) {
    first = std::ranges::find_if(episode, tokens.end(), [](const Token& token) {
      return is_open_bracket_token(token) && token.value == "「";
    });
    if (first != tokens.end()) ++first;
    last = std::find_if(first, tokens.end(), [](const Token& token) {
      return is_close_bracket_token(token) && token.value == "」";
    });
    if (last == tokens.end()) return {};
    if (std::ranges::any_of(first, last, is_identified_token)) return {};
  }

  return {first, last};
}

inline std::optional<Element> parse_episode_title(std::span<Token> tokens) noexcept {
  const auto span = find_episode_title(tokens);
  if (span.empty()) return {};

  std::string value = build_element_value(span, KeepDelimiters::No);
  if (value.empty()) return {};

  // Avoid single-character episode titles
  if (value.size() == 1) return {};

  for (auto& token : span) {
    token.element_kind = ElementKind::EpisodeTitle;
  }

  return Element{
      .kind = ElementKind::EpisodeTitle,
      .value = std::move(value),
      .position = span.front().position,
  };
}

}  // namespace anitomy::detail
