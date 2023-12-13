#pragma once

#include <optional>
#include <ranges>
#include <vector>

#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_file_extension(std::vector<Token>& tokens_) noexcept {
  if (tokens_.size() < 2) return std::nullopt;

  auto tokens = tokens_ | std::views::reverse;
  auto& last_token = tokens[0];
  auto& prev_token = tokens[1];

  if (last_token.keyword && last_token.keyword->kind == KeywordKind::FileExtension) {
    if (is_delimiter_token(prev_token) && prev_token.value == ".") {
      last_token.element_kind = ElementKind::FileExtension;
      return Element{
          .kind = ElementKind::FileExtension,
          .value = last_token.value,
      };
    }
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
