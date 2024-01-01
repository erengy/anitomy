#pragma once

#include <optional>
#include <ranges>
#include <span>

#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

inline std::optional<Element> parse_file_extension(std::span<Token> tokens) noexcept {
  static constexpr auto is_file_extension = [](const Token& token) {
    return token.keyword && token.keyword->kind == KeywordKind::FileExtension;
  };

  static constexpr auto is_dot = [](const Token& token) {
    return is_delimiter_token(token) && token.value == ".";
  };

  if (tokens.size() < 2) return std::nullopt;

  auto view = tokens | std::views::reverse | std::views::adjacent<2>;
  auto [last_token, prev_token] = view.front();

  if (!is_file_extension(last_token) || !is_dot(prev_token)) return std::nullopt;

  last_token.element_kind = ElementKind::FileExtension;

  return Element{
      .kind = ElementKind::FileExtension,
      .value = last_token.value,
      .position = last_token.position,
  };
}

}  // namespace anitomy::detail
