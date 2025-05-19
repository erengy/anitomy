#pragma once

#include <ranges>
#include <span>

#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_part(std::span<Token> tokens) noexcept {
  static constexpr auto is_part_keyword = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::Part;
  };

  auto view = tokens | std::views::filter(is_part_keyword);

  for (auto it = view.begin(); it != view.end(); ++it) {
    auto next_token = find_next_token(tokens, it.base(), is_not_delimiter_token);

    if (next_token == tokens.end()) continue;
    if (!is_numeric_token(*next_token)) continue;

    it->element_kind = ElementKind::Part;
    next_token->element_kind = ElementKind::Part;

    return element_from_token(ElementKind::Part, *next_token);
  }

  return {};
}

}  // namespace anitomy::detail
