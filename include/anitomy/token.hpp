#pragma once

#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "element.hpp"
#include "keyword.hpp"

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
  bool is_enclosed = false;  // token is enclosed in brackets
  bool is_number = false;    // all characters in `value` are digits
};

class TokenContainer {
public:
  using container_t = std::vector<Token>;
  using iterator_t = container_t::iterator;
  using predicate_t = std::function<bool(const Token&)>;

  explicit TokenContainer(container_t& tokens) : tokens_{std::move(tokens)} {
  }

  [[nodiscard]] constexpr auto&& tokens(this auto&& self) noexcept {
    return std::forward<decltype(self)>(self).tokens_;
  }

protected:
  inline iterator_t find_prev_token(iterator_t it, predicate_t predicate) noexcept {
    auto [token, end] = std::ranges::find_last_if(tokens_.begin(), it, predicate);
    return token;
  }

  inline iterator_t find_next_token(iterator_t it, predicate_t predicate) noexcept {
    if (it == tokens_.end()) return tokens_.end();
    return std::ranges::find_if(std::next(it), tokens_.end(), predicate);
  }

  container_t tokens_;
};

// A free token is an unidentified text token
static constexpr bool is_free_token(const Token& token) noexcept {
  return token.kind == TokenKind::Text && !token.element_kind;
}

static constexpr bool is_open_bracket_token(const Token& token) noexcept {
  return token.kind == TokenKind::OpenBracket;
}

static constexpr bool is_delimiter_token(const Token& token) noexcept {
  return token.kind == TokenKind::Delimiter;
}

static constexpr bool is_not_delimiter_token(const Token& token) noexcept {
  return token.kind != TokenKind::Delimiter;
};

static constexpr bool is_keyword_token(const Token& token) noexcept {
  return token.kind == TokenKind::Keyword;
}

static constexpr bool is_numeric_token(const Token& token) noexcept {
  return token.is_number;
}

}  // namespace anitomy::detail
