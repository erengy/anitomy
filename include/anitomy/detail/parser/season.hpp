#pragma once

#include <ranges>
#include <regex>
#include <span>
#include <tuple>
#include <vector>

#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_season(std::span<Token> tokens) noexcept {
  using window_t = std::tuple<Token&, Token&, Token&>;

  std::vector<Element> elements;

  {
    static constexpr auto is_season_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::Season;
    };

    static constexpr auto starts_with_season_keyword = [](window_t tokens) {
      return is_season_keyword(std::get<0>(tokens)) &&   //
             is_delimiter_token(std::get<1>(tokens)) &&  //
             is_free_token(std::get<2>(tokens));
    };

    static constexpr auto ends_with_season_keyword = [](window_t tokens) {
      return is_season_keyword(std::get<2>(tokens)) &&   //
             is_delimiter_token(std::get<1>(tokens)) &&  //
             is_free_token(std::get<0>(tokens));
    };

    for (auto view : tokens | std::views::adjacent<3>) {
      // Check previous token for a number (e.g. `2nd Season`)
      if (ends_with_season_keyword(view)) {
        auto [token, _, season_token] = view;
        if (auto number = from_ordinal_number(token.value); !number.empty()) {
          token.element_kind = ElementKind::Season;
          season_token.element_kind = ElementKind::Season;
          elements.emplace_back(ElementKind::Season, std::string{number}, token.position);
          break;
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
          elements.emplace_back(ElementKind::Season, value, token.position);
          break;
        }
      }
    }
  }

  // Season pattern (e.g. `S2`, `S01-02`)
  if (elements.empty()) {
    static constexpr auto is_season = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"S(\\d{1,2})"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    auto view = tokens | std::views::filter(is_free_token);

    for (auto token = view.begin(); token != view.end(); ++token) {
      if (!is_season(*token, matches)) continue;

      token->element_kind = ElementKind::Season;
      elements.emplace_back(ElementKind::Season, matches[1].str(),
                            token->position + matches.position(1));

      auto next_token = std::next(token.base());
      if (next_token == tokens.end() || !is_dash_token(*next_token)) continue;
      if (++next_token == tokens.end()) continue;

      if (is_free_token(*next_token) && is_numeric_token(*next_token)) {
        next_token->element_kind = ElementKind::Season;
        elements.emplace_back(ElementKind::Season, next_token->value, next_token->position);
        break;
      }
    }
  }

  // Japanese counter pattern (e.g. `第2期`)
  if (elements.empty()) {
    static constexpr auto is_japanese_counter = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"(?:第)?(\\d{1,2})期"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | std::views::filter(is_free_token)) {
      if (is_japanese_counter(token, matches)) {
        token.element_kind = ElementKind::Season;
        elements.emplace_back(ElementKind::Season, matches[1].str(),
                              token.position + matches.position(1));
        break;
      }
    }
  }

  return elements;
}

}  // namespace anitomy::detail
