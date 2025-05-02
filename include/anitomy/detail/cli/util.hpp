#pragma once

#include <ranges>
#include <string>
#include <vector>

#include <anitomy/detail/format.hpp>
#include <anitomy/detail/json.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

using row_t = std::vector<std::string>;

constexpr bool is_trivial_token(const Token& token) noexcept {
  using enum TokenKind;

  switch (token.kind) {
    case OpenBracket:
    case CloseBracket:
    case Delimiter:
      return true;
    default:
      return false;
  };
}

inline json::Value to_json(const std::vector<Element>& elements) {
  json::Value items{json::Value::object_t{}};

  for (const auto& element : elements) {
    const auto kind = std::string{to_string(element.kind)};

    if (items.as_object().contains(kind)) continue;

    const auto values =
        elements |
        std::views::filter([&element](const auto& e) { return e.kind == element.kind; }) |
        std::views::transform([](const auto& e) { return e.value; }) |
        std::ranges::to<json::Value::array_t>();

    if (values.size() == 1) {
      items.as_object().emplace(kind, values.front());
    } else {
      items.as_object().emplace(kind, values);
    }
  }

  return items;
}

inline json::Value to_json(const std::vector<Token>& tokens, bool verbose) {
  json::Value items{json::Value::array_t{}};

  for (const auto& token : tokens) {
    if (!verbose && is_trivial_token(token)) continue;

    items.as_array().emplace_back(token.value);
  }

  return items;
}

inline std::vector<row_t> to_rows(const std::vector<Element>& elements) {
  std::vector<row_t> rows;

  for (const auto& element : elements) {
    std::string kind{to_string(element.kind)};
    rows.emplace_back(row_t{kind, element.value});
  }

  return rows;
}

inline std::vector<row_t> to_rows(const std::vector<Token>& tokens, bool verbose) {
  std::vector<row_t> rows;

  for (const auto& token : tokens) {
    if (!verbose && is_trivial_token(token)) continue;

    rows.emplace_back(row_t{
        std::string{to_string(token.kind)},
        std::string{token.keyword ? to_string(token.keyword->kind) : ""},
        std::string{token.element_kind ? to_string(*token.element_kind) : ""},
        token.value,
    });
  }

  return rows;
}

}  // namespace anitomy::detail
