#pragma once

#include <ranges>
#include <regex>
#include <span>
#include <vector>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/delimiter.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_episode(std::span<Token> tokens) noexcept {
  using namespace std::views;

  std::vector<Element> elements;

  static const auto add_element = [&elements](ElementKind kind, std::string_view value,
                                              size_t position) {
    elements.emplace_back(kind, std::string{value}, position);
  };

  static const auto add_element_from_token = [&elements](ElementKind kind, Token& token,
                                                         std::string_view value = {},
                                                         size_t position = std::string::npos) {
    token.element_kind = kind;
    elements.emplace_back(kind, value.empty() ? token.value : std::string{value},
                          position != std::string::npos ? position : token.position);
  };

  // Episode prefix (e.g. `E1`, `EP1`, `Episode 1`)
  {
    static constexpr auto is_episode_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::Episode;
    };

    auto episode_token = std::ranges::find_if(tokens, is_episode_keyword);

    // Check next token for a number
    if (auto token = find_next_token(tokens, episode_token, is_not_delimiter_token);
        token != tokens.end()) {
      if (is_free_token(*token) && is_numeric_token(*token)) {
        add_element_from_token(ElementKind::Episode, *token);
        episode_token->element_kind = ElementKind::Episode;
        return elements;
      }
    }
  }
  {
    static constexpr auto is_episode_prefix = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{R"((?:E|E[Pp]|Eps)(\d{1,4})(?:[vV](\d))?)"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_episode_prefix(token, matches)) {
        add_element_from_token(ElementKind::Episode, token, matches[1].str(),
                               token.position + matches.position(1));
        if (matches[2].matched) {
          add_element(ElementKind::ReleaseVersion, matches[2].str(),
                      token.position + matches.position(2));
        }
        return elements;
      }
    }
  }

  // Number comes before another number (e.g. `8 & 10`, `01 of 24`)
  {
    auto view = tokens | filter(is_free_token) | filter(is_numeric_token);

    for (auto it = view.begin(); it != view.end(); ++it) {
      // skip if delimiter but not '&'
      auto token = std::ranges::find_if(
          std::next(it.base().base()), tokens.end(),
          [](const Token& token) { return is_not_delimiter_token(token) || token.value == "&"; });
      if (token == tokens.end()) continue;
      // check if '&' or "of"
      if (token->value != "&" && token->value != "of") continue;
      // skip if delimiter
      auto next_token = find_next_token(tokens, token, is_not_delimiter_token);
      if (next_token == tokens.end()) continue;
      // check if number
      if (!is_numeric_token(*next_token)) continue;
      add_element_from_token(ElementKind::Episode, *it);
      add_element_from_token(ElementKind::Episode, *next_token);
      return elements;
    }
  }

  // Single episode (e.g. `01v2`)
  {
    static constexpr auto is_single_episode = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{R"((\d{1,4})[vV](\d))"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_single_episode(token, matches)) {
        add_element_from_token(ElementKind::Episode, token, matches[1].str(),
                               token.position + matches.position(1));
        add_element(ElementKind::ReleaseVersion, matches[2].str(),
                    token.position + matches.position(2));
        return elements;
      }
    }
  }

  // Multi episode (e.g. `01-02`, `03-05v2`)
  {
    static constexpr auto is_multi_episode = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{R"((\d{1,4})(?:[vV](\d))?[-~&+])"
                                      R"((\d{1,4})(?:[vV](\d))?)"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_multi_episode(token, matches)) {
        auto lower = matches[1].str();
        auto upper = matches[3].str();
        if (to_int(lower) >= to_int(upper)) continue;  // avoid matching `009-1`, `5-2`, etc.
        add_element_from_token(ElementKind::Episode, token, lower,
                               token.position + matches.position(1));
        if (matches[2].matched) {
          add_element(ElementKind::ReleaseVersion, matches[2].str(),
                      token.position + matches.position(2));
        }
        add_element_from_token(ElementKind::Episode, token, upper,
                               token.position + matches.position(3));
        if (matches[4].matched) {
          add_element(ElementKind::ReleaseVersion, matches[4].str(),
                      token.position + matches.position(4));
        }
        return elements;
      }
    }
  }

  // Season and episode (e.g. `2x01`, `S01E03`, `S01-02xE001-150`)
  {
    static constexpr auto is_season_and_episode = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{
          "S?"
          "(\\d{1,2})(?:-S?(\\d{1,2}))?"
          "(?:x|[ ._-x]?E)"
          "(\\d{1,4})(?:-E?(\\d{1,4}))?"
          "(?:[vV](\\d))?"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_season_and_episode(token, matches)) {
        if (to_int(matches[1].str()) == 0) continue;
        add_element(ElementKind::Season, matches[1].str(), token.position + matches.position(1));
        if (matches[2].matched) {
          add_element(ElementKind::Season, matches[2].str(), token.position + matches.position(2));
        }
        add_element_from_token(ElementKind::Episode, token, matches[3].str(),
                               token.position + matches.position(3));
        if (matches[4].matched) {
          add_element(ElementKind::Episode, matches[4].str(), token.position + matches.position(4));
        }
        if (matches[5].matched) {
          add_element(ElementKind::ReleaseVersion, matches[5].str(),
                      token.position + matches.position(5));
        }
        return elements;
      }
    }
  }

  // Type and episode (e.g. `ED1`, `OP4a`, `OVA2`)
  {
    static constexpr auto is_type_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::Type;
    };

    auto type_token = std::ranges::find_if(tokens, is_type_keyword);

    // Check next token for a number
    if (auto token = find_next_token(tokens, type_token, is_not_delimiter_token);
        token != tokens.end()) {
      if (is_free_token(*token) && is_numeric_token(*token)) {
        add_element_from_token(ElementKind::Episode, *token);
        return elements;
      }
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

  // Number sign (e.g. `#01`, `#02-03v2`)
  {
    static constexpr auto is_number_sign = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"#(\\d{1,4})(?:[-~&+](\\d{1,4}))?(?:[vV](\\d))?"};
      return token.value.starts_with('#') && std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_number_sign(token, matches)) {
        add_element_from_token(ElementKind::Episode, token, matches[1].str(),
                               token.position + matches.position(1));
        if (matches[2].matched) {
          add_element(ElementKind::Episode, matches[2].str(), token.position + matches.position(2));
        }
        if (matches[3].matched) {
          add_element(ElementKind::ReleaseVersion, matches[3].str(),
                      token.position + matches.position(3));
        }
        return elements;
      }
    }
  }

  // Japanese counter (e.g. `第01話`)
  {
    static constexpr auto is_japanese_counter = [](const Token& token, std::smatch& matches) {
      static const std::regex pattern{"(?:第)?(\\d{1,4})話"};
      return std::regex_match(token.value, matches, pattern);
    };

    std::smatch matches;

    for (auto& token : tokens | filter(is_free_token)) {
      if (is_japanese_counter(token, matches)) {
        add_element_from_token(ElementKind::Episode, token, matches[1].str(),
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
    static constexpr auto is_dash_token = [](const Token& token) {
      return token.kind == TokenKind::Delimiter && is_dash(token.value.front());
    };

    auto view = tokens | filter(is_dash_token);

    for (auto it = view.begin(); it != view.end(); ++it) {
      auto next_token = std::ranges::find_if(it.base(), tokens.end(), is_not_delimiter_token);
      if (next_token != tokens.end() && is_numeric_token(*next_token)) {
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

    auto view = tokens | filter(is_free_token) | filter(is_partial_episode) | take(1);
    if (!view.empty()) {
      add_element_from_token(ElementKind::Episode, view.front());
      return elements;
    }
  }

  // Last number
  // @TODO: should not parse `1.11`, `Part 2`
  {
    auto view = tokens | reverse | filter(is_free_token) | filter(is_numeric_token) | take(1);

    if (!view.empty()) {
      add_element_from_token(ElementKind::Episode, view.front());
      return elements;
    }
  }

  return elements;
}

}  // namespace anitomy::detail
