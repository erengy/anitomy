#pragma once

#include <ranges>
#include <regex>
#include <span>
#include <vector>

#include <anitomy/detail/container.hpp>
#include <anitomy/detail/element.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_volume(std::span<Token> tokens) noexcept {
  std::vector<Element> elements;

  static constexpr auto is_volume_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::Volume;
  };

  while (true) {
    auto volume_token = std::ranges::find_if(tokens, is_volume_keyword);
    if (volume_token == tokens.end()) break;

    auto token = find_next_token(tokens, volume_token, is_not_delimiter_token);
    if (token == tokens.end() || !is_free_token(*token)) break;

    // Single volume (e.g. `01`, `01v2`)
    {
      static constexpr auto match_single_volume = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((\d{1,4})(?:[vV](\d))?)"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      if (match_single_volume(*token, matches)) {
        volume_token->element_kind = ElementKind::Volume;
        token->element_kind = ElementKind::Volume;
        elements.emplace_back(element_from_token(ElementKind::Volume, *token, matches.str(1)));
        if (matches[2].matched) {
          elements.emplace_back(ElementKind::ReleaseVersion, matches.str(2),
                                token->position + matches.position(2));
        }
      }
    }

    // Multiple volumes (e.g. `1&2`)
    {
      static constexpr auto match_multiple_volumes = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((\d{1,4})&(\d{1,4}))"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      if (match_multiple_volumes(*token, matches)) {
        volume_token->element_kind = ElementKind::Volume;
        token->element_kind = ElementKind::Volume;
        elements.emplace_back(element_from_token(ElementKind::Volume, *token, matches.str(1)));
        elements.emplace_back(element_from_token(ElementKind::Volume, *token, matches.str(2)));
      }
    }

    tokens = std::span<Token>(std::next(volume_token), tokens.end());
  }

  return elements;
}

}  // namespace anitomy::detail
