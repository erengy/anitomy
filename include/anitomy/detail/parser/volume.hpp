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

  static const auto add_element = [&elements](ElementKind kind, std::string_view value,
                                              size_t position) {
    elements.emplace_back(kind, std::string{value}, position);
  };

  static const auto add_element_from_token = [&elements](ElementKind kind, Token& token,
                                                         std::string_view value = {},
                                                         size_t position = std::string::npos) {
    token.element_kind = kind;
    elements.emplace_back(element_from_token(kind, token, value, position));
  };

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
      static constexpr auto is_single_volume = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((\d{1,4})(?:[vV](\d))?)"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      if (is_single_volume(*token, matches)) {
        volume_token->element_kind = ElementKind::Volume;
        add_element_from_token(ElementKind::Volume, *token, matches[1].str());
        if (matches[2].matched) {
          add_element(ElementKind::ReleaseVersion, matches[2].str(),
                      token->position + matches.position(2));
        }
      }
    }

    // @TODO: Multiple volumes (e.g. `Vol.1&2`)

    tokens = std::span<Token>(std::next(volume_token), tokens.end());
  }

  return elements;
}

}  // namespace anitomy::detail
