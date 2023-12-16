#pragma once

#include <string>
#include <string_view>

#include <anitomy/detail/unicode/utf32.hpp>
#include <anitomy/detail/unicode/utf8.hpp>

// As of C++23, the standard library still barely supports Unicode, despite SG16's efforts.
// We don't want to introduce a dependency either, so we roll our own code here.
//
// Note that this is not a general-purpose implementation and may not work properly in other
// contexts. Use at your own risk.
//
// References:
// - Unicode Standard: https://www.unicode.org/versions/latest/
// - Unicode FAQ: https://www.unicode.org/faq/utf_bom.html
// - WG21 SG16 Unicode Study Group: https://github.com/sg16-unicode/sg16
// - WHATWG Encoding Standard: https://encoding.spec.whatwg.org/
// - RFC 3629: https://datatracker.ietf.org/doc/html/rfc3629

namespace anitomy::detail::unicode {

[[nodiscard]] constexpr std::u32string utf8_to_utf32(std::string_view input) noexcept {
  std::u32string output;

  output.reserve(input.size());

  for (auto it = input.begin(); it != input.end();) {
    const auto result = utf8::decode(it, input.end());
    output.push_back(utf32::encode(result.code_point));
    it = result.next;
  }

  return output;
}

[[nodiscard]] constexpr std::string utf32_to_utf8(std::u32string_view input) noexcept {
  std::string output;

  output.reserve(input.size());

  for (auto it = input.begin(); it != input.end();) {
    const auto result = utf32::decode(it, input.end());
    output.append(utf8::encode(result.code_point));
    it = result.next;
  }

  return output;
}

}  // namespace anitomy::detail::unicode
