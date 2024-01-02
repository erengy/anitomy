#pragma once

#include <optional>
#include <ranges>
#include <regex>
#include <span>
#include <tuple>

#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_season(std::span<Token> tokens) noexcept {
  using window_t = std::tuple<Token&, Token&, Token&>;

  static constexpr auto is_season_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::Season;
  };

  static constexpr auto starts_with_season_keyword = [](window_t tokens) {
    // clang-format off
    return is_season_keyword(std::get<0>(tokens)) &&
           is_delimiter_token(std::get<1>(tokens)) &&
           is_free_token(std::get<2>(tokens));
    // clang-format on
  };

  static constexpr auto ends_with_season_keyword = [](window_t tokens) {
    // clang-format off
    return is_season_keyword(std::get<2>(tokens)) &&
           is_delimiter_token(std::get<1>(tokens)) &&
           is_free_token(std::get<0>(tokens));
    // clang-format on
  };

  for (auto view : tokens | std::views::adjacent<3>) {
    // Check previous token for a number (e.g. `2nd Season`)
    if (ends_with_season_keyword(view)) {
      auto [token, _, season_token] = view;
      if (auto number = from_ordinal_number(token.value); !number.empty()) {
        token.element_kind = ElementKind::Season;
        season_token.element_kind = ElementKind::Season;
        return Element{
            .kind = ElementKind::Season,
            .value = std::string{number},
            .position = token.position,
        };
      }
    }
    // Check next token for a number (e.g. `Season 2`, `Season II`)
    if (starts_with_season_keyword(view)) {
      auto [season_token, _, token] = view;
      std::string value;
      if (is_numeric_token(token)) {
        value = token.value;
      } else if (auto number = from_roman_number(token.value); !number.empty()) {
        value = number;
      }
      if (!value.empty()) {
        season_token.element_kind = ElementKind::Season;
        token.element_kind = ElementKind::Season;
        return Element{
            .kind = ElementKind::Season,
            .value = value,
            .position = token.position,
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

    for (auto& token : tokens | std::views::filter(is_free_token)) {
      if (is_season(token, matches) || is_japanese_counter(token, matches)) {
        token.element_kind = ElementKind::Season;
        return Element{
            .kind = ElementKind::Season,
            .value = matches[1].str(),
            .position = token.position + matches.position(1),
        };
      }
    }
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
