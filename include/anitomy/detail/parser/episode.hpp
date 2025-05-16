#pragma once

#include <format>
#include <ranges>
#include <regex>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_episode(std::span<Token> tokens) noexcept {
  using namespace std::views;

  std::vector<Element> elements;

  const auto add_element_from_token = [&elements](ElementKind kind, Token& token,
                                                  std::string_view value = {},
                                                  size_t position = std::string::npos) {
    token.element_kind = kind;
    elements.emplace_back(element_from_token(kind, token, value, position));
  };

  {
    static constexpr auto match_episode_token = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{
          "(?:S(\\d{1,2})|(\\d{1,2})x)?"  // season
          "[E#]?(\\d{1,4})"               // episode
          "(?:[vV](\\d))?"                // version
      };
      return std::regex_match(token.value, matches, pattern);
    };

    static constexpr auto parse_matches = [](const std::smatch& matches, Token& token,
                                             std::vector<Element>& elements) {
      if (matches[1].matched) {
        elements.emplace_back(ElementKind::Season, matches.str(1),
                              token.position + matches.position(1));
      } else if (matches[2].matched) {
        elements.emplace_back(ElementKind::Season, matches.str(2),
                              token.position + matches.position(2));
      }

      token.element_kind = ElementKind::Episode;
      elements.emplace_back(element_from_token(ElementKind::Episode, token, matches.str(3),
                                               token.position + matches.position(3)));

      if (matches[4].matched) {
        elements.emplace_back(ElementKind::ReleaseVersion, matches.str(4),
                              token.position + matches.position(4));
      }
    };

    static constexpr auto is_episode_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::Episode;
    };

    static constexpr auto is_type_keyword = [](const Token& token) {
      return token.keyword && (token.keyword->kind == KeywordKind::Type ||
                               token.keyword->kind == KeywordKind::EpisodeType);
    };

    static constexpr auto is_episode_delimiter = [](const Token& token) {
      static const std::set<char> delimiters{'-', '~', '&', '+'};
      return is_delimiter_token(token) && delimiters.contains(token.value.front());
    };

    static constexpr auto starts_with_episode_or_type_keyword = [](std::span<Token>& tokens,
                                                                   std::span<Token>::iterator& it) {
      if (it == tokens.end()) return false;
      if (is_episode_keyword(*it)) return true;
      if (is_type_keyword(*it)) return it->value != "Movie";
      return false;
    };

    static constexpr auto is_episode_range = [](std::span<Token>& tokens,
                                                std::span<Token>::iterator& it,
                                                std::pair<std::smatch, std::smatch>& matches) {
      if (it == tokens.end()) return false;
      if (!is_episode_delimiter(*it)) return false;
      if (++it == tokens.end()) return false;
      if (!match_episode_token(*it, matches.second)) return false;
      if (to_int(matches.first.str(3)) >= to_int(matches.second.str(3))) {
        return false;  // avoid matching `009-1`, `5-2`, etc.
      }
      return true;
    };

    auto view = tokens | filter(is_free_token);

    for (auto token = view.begin(); token != view.end(); ++token) {
      std::pair<std::smatch, std::smatch> matches;

      if (!match_episode_token(*token, matches.first)) continue;

      auto prev_token = find_prev_token(tokens, token.base(), is_not_delimiter_token);
      auto next_token = std::next(token.base());

      // A numeric token (e.g. `1`) is valid only if preceded by a keyword
      // (e.g. `Episode 1`, `OVA1`) or is part of a range (e.g. `1-12`).
      bool valid = !is_numeric_token(*token);
      valid = starts_with_episode_or_type_keyword(tokens, prev_token) || valid;
      valid = is_episode_range(tokens, next_token, matches) || valid;
      if (!valid) continue;

      parse_matches(matches.first, *token, elements);
      if (!matches.second.empty()) {
        parse_matches(matches.second, *next_token, elements);
      }
    }

    if (!elements.empty()) return elements;
  }

  // Separated episodes (e.g. `8 & 10`, `1 ~ 12`, `01 of 24`)
  {
    static constexpr auto is_separator = [](const Token& token) {
      return token.value == "&" || token.value == "~" || token.value == "of";
    };

    auto view = tokens | filter(is_free_token) | filter(is_numeric_token);

    for (auto it = view.begin(); it != view.end(); ++it) {
      auto next_token = std::next(it.base().base());
      auto token = std::ranges::find_if(next_token, tokens.end(), is_separator);
      if (token == tokens.end()) continue;
      if (std::ranges::any_of(next_token, token, is_not_delimiter_token)) continue;

      next_token = find_next_token(tokens, token, is_not_delimiter_token);
      if (next_token == tokens.end()) continue;
      if (!is_numeric_token(*next_token)) continue;

      add_element_from_token(ElementKind::Episode, *it);
      if (token->value != "of") {
        add_element_from_token(ElementKind::Episode, *next_token);
      }
      return elements;
    }
  }

  // Fractional episode (e.g. `07.5`)
  {
    for (auto [number, delimiter, fraction] : tokens | adjacent<3>) {
      if (is_free_token(number) && is_numeric_token(number)) {
        if (is_delimiter_token(delimiter) && delimiter.value == ".") {
          // We don't allow any fractional part other than `.5`, because there are cases
          // where such a number is a part of the title (e.g. `Evangelion: 1.11`,
          // `Tokyo Magnitude 8.0`) or a keyword (e.g. `5.1`).
          if (is_free_token(fraction) && fraction.value == "5") {
            add_element_from_token(
                ElementKind::Episode, number,
                std::format("{}{}{}", number.value, delimiter.value, fraction.value));
            delimiter.element_kind = ElementKind::Episode;
            fraction.element_kind = ElementKind::Episode;
            return elements;
          }
        }
      }
    }
  }

  // Japanese counter (e.g. `第01話`)
  {
    static constexpr auto match_japanese_counter = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"(?:第)?(\\d{1,4})話"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (match_japanese_counter(token, matches)) {
        add_element_from_token(ElementKind::Episode, token, matches.str(1),
                               token.position + matches.position(1));
        return elements;
      }
    }
  }

  // Equivalent numbers (e.g. `01 (176)`, `29 (04)`)
  {
    // @TODO
  }

  // Separated number (e.g. ` - 08`)
  {
    auto view = tokens | reverse | filter(is_dash_token);

    for (auto it = view.begin(); it != view.end(); ++it) {
      auto next_token =
          std::ranges::find_if(it.base().base(), tokens.end(), is_not_delimiter_token);
      if (next_token != tokens.end() && is_free_token(*next_token) &&
          is_numeric_token(*next_token)) {
        add_element_from_token(ElementKind::Episode, *next_token);
        return elements;
      }
    }
  }

  // Isolated number (e.g. `[12]`, `(2006)`)
  {
    using window_t = std::tuple<Token&, Token&, Token&>;

    static constexpr auto is_isolated = [](window_t tokens) {
      return std::get<0>(tokens).kind == TokenKind::OpenBracket &&
             std::get<2>(tokens).kind == TokenKind::CloseBracket;
    };

    static constexpr auto is_free_number = [](window_t tokens) {
      auto& token = std::get<1>(tokens);
      return is_free_token(token) && is_numeric_token(token);
    };

    auto view = tokens | adjacent<3> | filter(is_isolated) | filter(is_free_number) | take(1);

    if (!view.empty()) {
      add_element_from_token(ElementKind::Episode, std::get<1>(view.front()));
      return elements;
    }
  }

  // Partial episode (e.g. `4a`, `111C`)
  {
    static constexpr auto is_partial_episode = [](const Token& token) {
      static const std::regex pattern{R"(\d{1,4}[ABCabc])"};
      return std::regex_match(token.value, pattern);
    };

    static constexpr auto _ = [](auto pred) {
      return [pred](const auto& tuple) { return pred(std::get<1>(tuple)); };
    };

    auto view = tokens | enumerate | filter(_(is_free_token)) | filter(_(is_partial_episode));

    for (auto [i, token] : view) {
      if (i > 1 && tokens[i - 2].value == "Ver1" && token.value == "1a") {
        continue;  // `NieR:Automata Ver1.1a`
      }

      add_element_from_token(ElementKind::Episode, token);
      return elements;
    }
  }

  // Last number
  {
    static constexpr auto is_version_number = [](auto token) {
      if (!is_numeric_token(*token)) return false;
      const auto it = std::prev(token);
      return is_delimiter_token(*it) && it->value == ".";  // e.g. `1.1`, `3.33`
    };

    auto view = tokens | reverse | filter(is_free_token) | filter(is_numeric_token);

    for (auto token = view.begin(); token != view.end(); ++token) {
      if (token->is_enclosed) continue;

      const auto prev_token =
          find_next_token(token.base().base(), tokens.rend(), is_not_delimiter_token);
      const auto next_token =
          find_next_token(token.base().base().base(), tokens.end(), is_not_delimiter_token);

      if (prev_token != tokens.rend()) {
        if (prev_token->value == "Part") continue;  // e.g. `Part 2`
        if (is_version_number(prev_token)) continue;
      }
      if (next_token != tokens.end()) {
        if (is_version_number(next_token)) continue;
      }
      if (prev_token != tokens.rend() && next_token != tokens.end()) {
        if (is_free_token(*prev_token) && is_free_token(*next_token)) continue;
      }

      add_element_from_token(ElementKind::Episode, *token);
      return elements;
    }
  }

  return elements;
}

}  // namespace anitomy::detail
