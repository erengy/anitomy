#pragma once

#include <optional>
#include <ranges>
#include <regex>
#include <tuple>
#include <vector>

#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_anime_season(std::vector<Token>& tokens_) noexcept {
  using namespace std::views;
  using window_t = std::tuple<Token&, Token&, Token&>;

  static constexpr auto is_anime_season_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::AnimeSeason;
  };

  static constexpr auto starts_with_season_keyword = [](window_t tokens) {
    return is_anime_season_keyword(std::get<0>(tokens)) &&
           is_delimiter_token(std::get<1>(tokens)) && is_free_token(std::get<2>(tokens));
  };

  static constexpr auto ends_with_season_keyword = [](window_t tokens) {
    return is_anime_season_keyword(std::get<2>(tokens)) &&
           is_delimiter_token(std::get<1>(tokens)) && is_free_token(std::get<0>(tokens));
  };

  for (auto tokens : tokens_ | std::views::adjacent<3>) {
    // Check previous token for a number (e.g. `2nd Season`)
    if (ends_with_season_keyword(tokens)) {
      auto [token, _, season_token] = tokens;
      if (auto number = from_ordinal_number(token.value); !number.empty()) {
        token.element_kind = ElementKind::AnimeSeason;
        season_token.element_kind = ElementKind::AnimeSeason;
        return Element{
            .kind = ElementKind::AnimeSeason,
            .value = std::string{number},
        };
      }
    }
    // Check next token for a number (e.g. `Season 2`, `Season II`)
    if (starts_with_season_keyword(tokens)) {
      auto [season_token, _, token] = tokens;
      if (is_numeric_token(token)) {
        season_token.element_kind = ElementKind::AnimeSeason;
        token.element_kind = ElementKind::AnimeSeason;
        return Element{
            .kind = ElementKind::AnimeSeason,
            .value = token.value,
        };
      } else if (auto number = from_roman_number(token.value); !number.empty()) {
        season_token.element_kind = ElementKind::AnimeSeason;
        token.element_kind = ElementKind::AnimeSeason;
        return Element{
            .kind = ElementKind::AnimeSeason,
            .value = std::string{number},
        };
      }
    }
  }

  // Other season patterns (e.g. `S2`, `第2期`)
  {
    static constexpr auto is_season = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"S(\\d{1,2})"};
      return std::regex_match(token.value, matches, pattern);
    };

    static constexpr auto is_japanese_counter = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"(?:第)?(\\d{1,2})期"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens_ | filter(is_free_token)) {
      if (is_season(token, matches) || is_japanese_counter(token, matches)) {
        token.element_kind = ElementKind::AnimeSeason;
        return Element{
            .kind = ElementKind::AnimeSeason,
            .value = matches[1].str(),
        };
      }
    }
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
