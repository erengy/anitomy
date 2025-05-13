#pragma once

#include <algorithm>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <anitomy/detail/bracket.hpp>
#include <anitomy/detail/delimiter.hpp>
#include <anitomy/detail/keyword.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/detail/unicode.hpp>
#include <anitomy/detail/util.hpp>
#include <anitomy/options.hpp>

namespace anitomy::detail {

class Tokenizer final {
public:
  // Input must be UTF-8 encoded and should be in composed form (NFC/NFKC).
  // UTF-32 is used internally for easier processing.
  constexpr explicit Tokenizer(std::string_view input) noexcept
      : input_{unicode::utf8_to_utf32(input)}, view_{input_} {
  }

  constexpr void tokenize(const Options&) noexcept {
    while (auto token = next_token()) {
      tokens_.emplace_back(*token);
    }
    process_tokens();
  }

  [[nodiscard]] constexpr auto&& tokens(this auto&& self) noexcept {
    return std::forward<decltype(self)>(self).tokens_;
  }

private:
  [[nodiscard]] constexpr std::optional<Token> next_token() noexcept {
    if (is_eof()) {
      return std::nullopt;
    }

    if (is_open_bracket(peek())) {
      return Token{
          .kind = TokenKind::OpenBracket,
          .value = take(),
      };
    }
    if (is_close_bracket(peek())) {
      return Token{
          .kind = TokenKind::CloseBracket,
          .value = take(),
      };
    }

    if (is_delimiter(peek())) {
      return Token{
          .kind = TokenKind::Delimiter,
          .value = take(),
      };
    }

    if (auto [value, keyword] = take_keyword(); !value.empty()) {
      return Token{
          .kind = TokenKind::Keyword,
          .value = value,
          .keyword = keyword,
      };
    }

    return Token{
        .kind = TokenKind::Text,
        .value = take_text(),
    };
  }

  constexpr void process_tokens() noexcept {
    int bracket_level = 0;
    size_t position = 0;

    for (auto& token : tokens_) {
      if (token.kind == TokenKind::OpenBracket) {
        bracket_level += 1;
      } else if (token.kind == TokenKind::CloseBracket) {
        bracket_level -= 1;
      } else {
        token.is_enclosed = bracket_level > 0;
      }

      token.position = position;
      position += token.value.size();

      if (token.kind == TokenKind::Text) {
        token.is_number = std::ranges::all_of(token.value, is_digit<char>);
      }
    }
  }

  [[nodiscard]] static constexpr bool is_text(const char32_t ch) noexcept {
    return !is_bracket(ch) && !is_delimiter(ch);
  }

  [[nodiscard]] static constexpr bool is_word_boundary(const char32_t ch) noexcept {
    return !is_text(ch);
  }

  [[nodiscard]] constexpr bool is_eof() const noexcept {
    return view_.empty();
  }

  [[nodiscard]] constexpr char32_t peek() const noexcept {
    return view_.front();
  }

  [[nodiscard]] constexpr std::u32string_view peek(
      const size_t offset, const size_t n = std::u32string_view::npos) const noexcept {
    return view_.substr(offset, n);
  }

  [[nodiscard]] constexpr std::string take(const size_t n = 1) noexcept {
    auto view = view_ | std::views::take(n);
    view_.remove_prefix(n);
    return unicode::utf32_to_utf8(view);
  }

  [[nodiscard]] constexpr std::string take_text() noexcept {
    auto text = view_ | std::views::take_while(is_text);
    auto n = std::ranges::distance(text);
    return take(n);
  }

  [[nodiscard]] inline std::pair<std::string, Keyword> take_keyword() noexcept {
    static constexpr auto has_candidates = [](std::string_view prefix) {
      static constexpr auto keys = keywords | std::views::keys;
      const auto starts_with = [&prefix](std::string_view keyword) {
        return std::ranges::starts_with(keyword, prefix, equal_to);
      };
      return std::ranges::find_if(keys, starts_with) != keys.end();
    };

    static constexpr auto find_key = [](const std::u32string_view view) {
      std::string key;
      for (size_t n = 1; n <= view.size(); ++n) {
        const auto prefix = unicode::utf32_to_utf8(view.substr(0, n));
        if (keywords.contains(prefix)) key = prefix;
        if (!has_candidates(prefix)) break;
      }
      return key;
    };

    static constexpr auto is_keyword_boundary = [](const Keyword& keyword,
                                                   const std::u32string_view view) {
      if (keyword.is_subword()) return true;
      if (view.empty()) return true;
      const auto next = view.front();
      if (is_word_boundary(next)) return true;
      if (keyword.is_prefix_for_number()) return is_digit(next);
      if (keyword.is_prefix_for_other()) return !find_key(view).empty();
      return false;
    };

    const std::string key = find_key(view_);

    if (key.empty()) return {};

    const size_t n = key.size();
    const auto keyword = keywords[key];

    if (!is_keyword_boundary(keyword, peek(n))) return {};

    return {take(n), keyword};
  }

  std::u32string input_;
  std::u32string_view view_;
  std::vector<Token> tokens_;
};

}  // namespace anitomy::detail
