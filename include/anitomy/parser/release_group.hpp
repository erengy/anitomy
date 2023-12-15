#pragma once

#include <optional>
#include <ranges>
#include <span>

#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_release_group(std::span<Token> tokens) noexcept {
  auto token_begin = tokens.begin();
  auto token_end = tokens.begin();

  do {
    // Find the first free enclosed token
    token_begin = std::ranges::find_if(token_end, tokens.end(), [](const Token& token) {
      return is_free_token(token) && token.is_enclosed;
    });
    if (token_begin == tokens.end()) break;

    // Continue until a bracket or identifier is found
    token_end = std::ranges::find_if(token_begin, tokens.end(), [](const Token& token) {
      return token.kind == TokenKind::CloseBracket || token.element_kind.has_value();
    });
    if (token_end == tokens.end()) break;
    if (token_end->kind != TokenKind::CloseBracket) continue;

    // @TODO: Ignore if it's not the first non-delimiter token in group

    auto span = std::span(token_begin, token_end);

    // Build release group
    if (std::string value = build_element_value(span, false); !value.empty()) {
      for (auto& token : span) {
        token.element_kind = ElementKind::ReleaseGroup;
      }
      return Element{
          .kind = ElementKind::ReleaseGroup,
          .value = value,
      };
    }
    break;
  } while (token_begin != tokens.end());

  return std::nullopt;
}

}  // namespace anitomy::detail
