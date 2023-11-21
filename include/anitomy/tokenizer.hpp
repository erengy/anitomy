#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "keyword.hpp"
#include "options.hpp"
#include "token.hpp"
#include "unicode.hpp"
#include "util.hpp"

namespace anitomy::detail {

class Tokenizer final {
public:
  // Input must be UTF-8 encoded and should be in composed form (NFC/NFKC).
  // UTF-32 is used internally for easier processing.
  constexpr explicit Tokenizer(std::string_view input) noexcept
      : input_{unicode::utf8_to_utf32(input)}, view_{input_} {
  }

  constexpr void tokenize(const Options& options) noexcept {
    while (auto token = next_token()) {
      tokens_.emplace_back(*token);
    }
    process_tokens();
  }

  [[nodiscard]] constexpr std::vector<Token>& tokens() noexcept {
    return tokens_;
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

    if (auto delimiter_kind = to_delimiter_kind(peek())) {
      return Token{
          .kind = TokenKind::Delimiter,
          .value = take(),
          .delimiter_kind = *delimiter_kind,
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

    for (auto& token : tokens_) {
      if (token.kind == TokenKind::OpenBracket) {
        bracket_level += 1;
      } else if (token.kind == TokenKind::CloseBracket) {
        bracket_level -= 1;
      } else {
        token.is_enclosed = bracket_level > 0;
      }

      if (token.kind == TokenKind::Text) {
        token.is_number = std::ranges::all_of(token.value, is_digit);
      }
    }
  }

  [[nodiscard]] static constexpr bool is_open_bracket(const char32_t ch) noexcept {
    switch (ch) {
      case U'(':       // parenthesis
      case U'[':       // square bracket
      case U'{':       // curly bracket
      case U'\u300C':  // corner bracket
      case U'\u300E':  // white corner bracket
      case U'\u3010':  // black lenticular bracket
      case U'\uFF08':  // fullwidth parenthesis
      case U'\uFF3B':  // fullwidth square bracket
      case U'\uFF5B':  // fullwidth curly bracket
        return true;
      default:
        return false;
    }
  }

  [[nodiscard]] static constexpr bool is_close_bracket(const char32_t ch) noexcept {
    switch (ch) {
      case U')':       // parenthesis
      case U']':       // square bracket
      case U'}':       // curly bracket
      case U'\u300D':  // corner bracket
      case U'\u300F':  // white corner bracket
      case U'\u3011':  // black lenticular bracket
      case U'\uFF09':  // fullwidth parenthesis
      case U'\uFF3D':  // fullwidth square bracket
      case U'\uFF5D':  // fullwidth curly bracket
        return true;
      default:
        return false;
    }
  }

  [[nodiscard]] static constexpr bool is_bracket(const char32_t ch) noexcept {
    return is_open_bracket(ch) || is_close_bracket(ch);
  }

  [[nodiscard]] static constexpr std::optional<DelimiterKind> to_delimiter_kind(
      const char32_t ch) noexcept {
    switch (ch) {
      case U' ':       // space
      case U'\t':      // character tabulation
      case U'\u00A0':  // no-break space
      case U'\u200B':  // zero width space
      case U'\u3000':  // ideographic space
        return DelimiterKind::Space;

      case U'-':       // hyphen-minus
      case U'\u00AD':  // soft hyphen
      case U'\u2010':  // hyphen
      case U'\u2011':  // non-breaking hyphen
      case U'\u2012':  // figure dash
      case U'\u2013':  // en dash
      case U'\u2014':  // em dash
      case U'\u2015':  // horizontal bar
        return DelimiterKind::Dash;

      case U'_':  // used instead of space
      case U'.':  // used instead of space, problematic (e.g. `AAC2.0.H.264`)
      case U',':  // used to separate keywords
      case U'&':  // used for episode ranges
      case U'+':  // used in torrent titles
      case U'|':  // used in torrent titles, reserved in Windows
        return DelimiterKind::Other;

      default:
        return std::nullopt;
    }
  }

  [[nodiscard]] static constexpr bool is_delimiter(const char32_t ch) noexcept {
    return to_delimiter_kind(ch).has_value();
  }

  [[nodiscard]] static constexpr bool is_text(const char32_t ch) noexcept {
    return !is_bracket(ch) && !is_delimiter(ch);
  }

  [[nodiscard]] static constexpr bool is_word_boundary(const char32_t ch) noexcept {
    return is_bracket(ch) || is_delimiter(ch);
  }

  [[nodiscard]] constexpr bool is_eof() const noexcept {
    return view_.empty();
  }

  [[nodiscard]] constexpr char32_t peek() const noexcept {
    return view_.front();
  }

  [[nodiscard]] constexpr std::u32string_view peek(const size_t n) const noexcept {
    return view_.substr(0, n);
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
    static constexpr auto count_candidates = [](std::string_view prefix) {
      static const auto keys = keywords | std::views::keys;
      return std::ranges::count_if(keys, [&prefix](std::string_view keyword) {
        return std::ranges::starts_with(keyword, prefix, equal_to);
      });
    };

    static constexpr auto is_keyword_boundary = [](std::u32string_view view) {
      return view.empty() || is_word_boundary(view.front());
    };

    std::string key;

    for (size_t n = 1; n <= view_.size(); ++n) {
      auto prefix = unicode::utf32_to_utf8(peek(n));
      if (keywords.contains(prefix)) key = prefix;
      if (count_candidates(prefix) > 0) continue;
      if (key.empty()) break;
    }

    if (key.empty()) return {};

    const size_t n = key.size();
    const auto keyword = keywords[key];

    if (keyword.is_bounded() && !is_keyword_boundary(view_.substr(n))) return {};

    return std::make_pair(take(n), keyword);
  }

  std::u32string input_;
  std::u32string_view view_;
  std::vector<Token> tokens_;
};

}  // namespace anitomy::detail
