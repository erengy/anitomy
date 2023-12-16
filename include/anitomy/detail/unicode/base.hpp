#pragma once

namespace anitomy::detail::unicode {

using byte_t = unsigned char;
using code_point_t = char32_t;

// Used to replace unknown or unrepresentable values
constexpr char32_t replacement_character = U'\uFFFD';

template <typename It>
struct DecodeResult {
  code_point_t code_point = replacement_character;
  bool error = false;
  It next;
};

// Surrogate code points `U+D800..U+DFFF` are reserved for use in UTF-16
[[nodiscard]] constexpr bool is_surrogate(const code_point_t cp) noexcept {
  return 0xD800 <= cp && cp <= 0xDFFF;
}

// Any Unicode code point except surrogates
[[nodiscard]] constexpr bool is_scalar_value(const code_point_t cp) noexcept {
  return cp <= 0x10FFFF && !is_surrogate(cp);
}

}  // namespace anitomy::detail::unicode
