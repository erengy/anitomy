#pragma once

namespace anitomy::detail {

[[nodiscard]] constexpr bool is_space(const char32_t ch) noexcept {
  switch (ch) {
    case U' ':       // space
    case U'\t':      // character tabulation
    case U'\u00A0':  // no-break space
    case U'\u200B':  // zero width space
    case U'\u3000':  // ideographic space
      return true;
    default:
      return false;
  }
}

[[nodiscard]] constexpr bool is_dash(const char32_t ch) noexcept {
  switch (ch) {
    case U'-':       // hyphen-minus
    case U'\u00AD':  // soft hyphen
    case U'\u2010':  // hyphen
    case U'\u2011':  // non-breaking hyphen
    case U'\u2012':  // figure dash
    case U'\u2013':  // en dash
    case U'\u2014':  // em dash
    case U'\u2015':  // horizontal bar
      return true;
    default:
      return false;
  }
}

[[nodiscard]] constexpr bool is_delimiter(const char32_t ch) noexcept {
  switch (ch) {
    case U'_':  // used instead of space
    case U'.':  // used instead of space, problematic (e.g. `AAC2.0.H.264`)
    case U',':  // used to separate keywords
    case U'&':  // used for episode ranges
    case U'+':  // used in torrent titles
    case U'|':  // used in torrent titles, reserved in Windows
      return true;
    default:
      return is_space(ch) || is_dash(ch);
  }
}

}  // namespace anitomy::detail
