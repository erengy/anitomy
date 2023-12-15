#pragma once

#include <optional>
#include <ranges>
#include <span>

#include "../container.hpp"
#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_volume_number(std::span<Token> tokens) noexcept {
  static constexpr auto is_volume_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::Volume;
  };

  auto volume_token = std::ranges::find_if(tokens, is_volume_keyword);

  // Check next token for a number
  if (auto token = find_next_token(tokens, volume_token, is_not_delimiter_token);
      token != tokens.end()) {
    if (is_free_token(*token) && is_numeric_token(*token)) {
      token->element_kind = ElementKind::VolumeNumber;
      volume_token->element_kind = ElementKind::VolumeNumber;
      return Element{
          .kind = ElementKind::VolumeNumber,
          .value = token->value,
      };
    }
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
