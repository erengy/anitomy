#pragma once

#include <optional>
#include <string>

#include <anitomy/detail/keyword.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

enum class TokenKind {
  OpenBracket,
  CloseBracket,
  Delimiter,
  Keyword,
  Text,
};

struct Token {
  TokenKind kind;
  std::string value;
  std::optional<Keyword> keyword;
  std::optional<ElementKind> element_kind;
  size_t position = 0;       // index in input string
  bool is_enclosed = false;  // token is enclosed in brackets
  bool is_number = false;    // all characters in `value` are digits
};

constexpr bool is_identified_token(const Token& token) noexcept {
  return token.element_kind.has_value();
};

constexpr bool is_free_token(const Token& token) noexcept {
  return (token.kind == TokenKind::Text || token.kind == TokenKind::Keyword) && !token.element_kind;
}

constexpr bool is_open_bracket_token(const Token& token) noexcept {
  return token.kind == TokenKind::OpenBracket;
}

constexpr bool is_close_bracket_token(const Token& token) noexcept {
  return token.kind == TokenKind::CloseBracket;
}

constexpr bool is_bracket_token(const Token& token) noexcept {
  return is_open_bracket_token(token) || is_close_bracket_token(token);
}

constexpr bool is_delimiter_token(const Token& token) noexcept {
  return token.kind == TokenKind::Delimiter;
}

constexpr bool is_not_delimiter_token(const Token& token) noexcept {
  return token.kind != TokenKind::Delimiter;
};

constexpr bool is_keyword_token(const Token& token) noexcept {
  return token.kind == TokenKind::Keyword;
}

constexpr bool is_numeric_token(const Token& token) noexcept {
  return token.is_number;
}

}  // namespace anitomy::detail
