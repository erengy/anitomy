#pragma once

#include <optional>
#include <ranges>
#include <vector>

#include "../container.hpp"
#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_volume_number(std::vector<Token>& tokens_) noexcept {
  static constexpr auto is_volume_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::Volume;
  };

  auto volume_token = std::ranges::find_if(tokens_, is_volume_keyword);

  // Check next token for a number
  if (auto token = find_next_token(tokens_, volume_token, is_not_delimiter_token);
      token != tokens_.end()) {
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
