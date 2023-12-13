#pragma once

#include <algorithm>
#include <functional>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "element.hpp"
#include "token.hpp"

namespace anitomy::detail {

class ElementContainer {
public:
  [[nodiscard]] constexpr auto&& elements(this auto&& self) noexcept {
    return std::forward<decltype(self)>(self).elements_;
  }

  [[nodiscard]] bool contains(ElementKind kind) const noexcept {
    const auto is_kind = [&kind](const Element& element) { return element.kind == kind; };
    return std::ranges::any_of(elements_, is_kind);
  }

protected:
  [[nodiscard]] static std::string build_element_value(
      const std::span<Token> tokens, const bool transform_delimiters = true) noexcept {
    std::string element_value;

    const bool has_multiple_delimiters = [&tokens]() {
      constexpr auto token_value = [](const Token& token) { return token.value.front(); };
      auto delimiters_view =
          tokens | std::views::filter(is_delimiter_token) | std::views::transform(token_value);
      std::set<char> delimiters{delimiters_view.begin(), delimiters_view.end()};
      return delimiters.size() > 1;
    }();

    const auto is_transformable = [&](const Token& token) {
      if (token.kind != TokenKind::Delimiter || !transform_delimiters) return false;
      switch (token.value.front()) {
        case ',':
        case '&':
          return false;
        case '_':
          return true;
        default:
          return !has_multiple_delimiters;
      }
    };

    for (const auto& token : tokens) {
      if (is_transformable(token)) {
        element_value.push_back(' ');
      } else {
        element_value.append(token.value);
      }
    }

    return element_value;
  }

  constexpr void add_element(ElementKind kind, std::string_view value) noexcept {
    elements_.emplace_back(kind, std::string{value});
  }

  constexpr void add_element_from_token(ElementKind kind, Token& token, std::string_view value = {},
                                        bool identify_token = true) noexcept {
    if (identify_token) token.element_kind = kind;
    elements_.emplace_back(kind, value.empty() ? token.value : std::string{value});
  }

  constexpr void add_element_from_tokens(ElementKind kind, std::span<Token> tokens,
                                         std::string_view value) noexcept {
    for (auto& token : tokens) {
      token.element_kind = kind;
    }
    elements_.emplace_back(kind, std::string{value});
  }

// private:
  std::vector<Element> elements_;
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

}  // namespace anitomy::detail
