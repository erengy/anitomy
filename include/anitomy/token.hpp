#pragma once

#include <optional>
#include <string>

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
  std::optional<KeywordKind> keyword_kind;
  std::optional<ElementKind> element_kind;
  bool is_enclosed = false;  // token is enclosed in brackets
  bool is_number = false;    // all characters in `value` are digits
};

}  // namespace anitomy::detail
