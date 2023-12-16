#pragma once

#include <array>
#include <string>

#include <anitomy/detail/unicode/base.hpp>

namespace anitomy::detail::unicode::utf8 {

// Continuation bytes are marked with `10` in their high-order bits
[[nodiscard]] constexpr bool is_continuation(const byte_t b) noexcept {
  return (b & 0b11000000) == 0b10000000;
}

// Well-formed UTF-8 byte sequences cannot contain these values
[[nodiscard]] constexpr bool is_invalid(const byte_t b) noexcept {
  return b == 0xC0 || b == 0xC1 || b >= 0xF5;
}

// Overlong (non-shortest form) sequences are ill-formed
[[nodiscard]] constexpr bool is_overlong(const code_point_t cp, const size_t length) noexcept {
  return (cp <= 0x7F && length > 1) || (cp <= 0x7FF && length > 2) || (cp <= 0xFFFF && length > 3);
}

[[nodiscard]] inline size_t sequence_length(const byte_t first_byte) noexcept {
  // See the branchless UTF-8 decoder by Chris Wellons:
  // https://github.com/skeeto/branchless-utf8
  static constexpr std::array<size_t, 1 << 5> sequence_lengths{
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0,
  };
  return sequence_lengths[first_byte >> 3];
}

[[nodiscard]] constexpr std::string encode(const code_point_t cp) noexcept {
  // U+0000..U+007F | 1-byte sequence
  if (cp <= 0x7F) {
    return {static_cast<char>(cp)};  // 0xxxxxxx
  }
  // U+0080..U+07FF | 2-byte sequence
  if (cp <= 0x7FF) {
    return {
        //                                      00000yyy'yyxxxxxx
        static_cast<char>(0b11000000 | ((cp & 0b00000111'11000000) >> 6)),  // 110yyyyy
        static_cast<char>(0b10000000 | ((cp & 0b00000000'00111111))),       // 10xxxxxx
    };
  }
  // U+0800..U+FFFF | 3-byte sequence
  // (excluding surrogate code points U+D800..U+DFFF)
  if (cp <= 0xFFFF && !is_surrogate(cp)) {
    return {
        //                                      zzzzyyyy'yyxxxxxx
        static_cast<char>(0b11100000 | ((cp & 0b11110000'00000000) >> 12)),  // 1110zzzz
        static_cast<char>(0b10000000 | ((cp & 0b00001111'11000000) >> 6)),   // 10yyyyyy
        static_cast<char>(0b10000000 | ((cp & 0b00000000'00111111))),        // 10xxxxxx
    };
  }
  // U+10000..U+10FFFF | 4-byte sequence
  if (0x10000 <= cp && cp <= 0x10FFFF) {
    return {
        //                                      000uuuuu'zzzzyyyy'yyxxxxxx
        static_cast<char>(0b11110000 | ((cp & 0b00011100'00000000'00000000) >> 18)),  // 11110uuu
        static_cast<char>(0b10000000 | ((cp & 0b00000011'11110000'00000000) >> 12)),  // 10uuzzzz
        static_cast<char>(0b10000000 | ((cp & 0b00000000'00001111'11000000) >> 6)),   // 10yyyyyy
        static_cast<char>(0b10000000 | ((cp & 0b00000000'00000000'00111111))),        // 10xxxxxx
    };
  }
  // Invalid scalar value
  return encode(replacement_character);
}

[[nodiscard]] constexpr code_point_t decode(const std::array<byte_t, 4> b,
                                            const size_t length) noexcept {
  switch (length) {
    // U+0000..U+007F | 1-byte sequence
    case 1:
    default:
      return static_cast<code_point_t>(b[0]);  // 0xxxxxxx

    // U+0080..U+07FF | 2-byte sequence | 00000yyy'yyxxxxxx
    case 2:
      return static_cast<code_point_t>((b[0] & 0b00011111) << 6) |  // 110yyyyy
             static_cast<code_point_t>((b[1] & 0b00111111));        // 10xxxxxx

    // U+0800..U+FFFF | 3-byte sequence | zzzzyyyy'yyxxxxxx
    case 3:
      return static_cast<code_point_t>((b[0] & 0b00001111) << 12) |  // 1110zzzz
             static_cast<code_point_t>((b[1] & 0b00111111) << 6) |   // 10yyyyyy
             static_cast<code_point_t>((b[2] & 0b00111111));         // 10xxxxxx

    // U+10000..U+10FFFF | 4-byte sequence | 000uuuuu'zzzzyyyy'yyxxxxxx
    case 4:
      return static_cast<code_point_t>((b[0] & 0b00000111) << 18) |  // 11110uuu
             static_cast<code_point_t>((b[1] & 0b00111111) << 12) |  // 10uuzzzz
             static_cast<code_point_t>((b[2] & 0b00111111) << 6) |   // 10yyyyyy
             static_cast<code_point_t>((b[3] & 0b00111111));         // 10xxxxxx
  }
}

template <typename It>
constexpr DecodeResult<It> decode(It it, It last) noexcept {
  if (it == last) {
    return {.error = true, .next = it};
  }

  const byte_t first_byte = static_cast<byte_t>(*it);
  const size_t length = sequence_length(first_byte);

  if (!length || is_continuation(first_byte) || is_invalid(first_byte)) {
    return {.error = true, .next = ++it};
  }

  if (length == 1) {
    return {
        .code_point = static_cast<code_point_t>(first_byte),
        .next = ++it,
    };
  }

  std::array<byte_t, 4> bytes{first_byte};
  ++it;
  for (size_t i = 1; i < length; ++i, ++it) {
    if (it == last) {
      return {.error = true, .next = it};
    }
    bytes[i] = static_cast<byte_t>(*it);
    if (!is_continuation(bytes[i]) || is_invalid(bytes[i])) {
      return {.error = true, .next = ++it};
    }
  }

  const code_point_t decoded = decode(bytes, length);

  if (is_overlong(decoded, length) || !is_scalar_value(decoded)) {
    return {.error = true, .next = it};
  }

  return {
      .code_point = decoded,
      .next = it,
  };
}

}  // namespace anitomy::detail::unicode::utf8
