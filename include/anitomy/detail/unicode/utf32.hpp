#pragma once

#include <anitomy/detail/unicode/base.hpp>

namespace anitomy::detail::unicode::utf32 {

[[nodiscard]] constexpr char32_t encode(const code_point_t cp) noexcept {
  return is_scalar_value(cp) ? cp : replacement_character;
}

template <typename It>
constexpr DecodeResult<It> decode(It it, It last) noexcept {
  if (it == last) {
    return {.error = true, .next = it};
  }

  const code_point_t decoded = static_cast<code_point_t>(*it);

  if (!is_scalar_value(decoded)) {
    return {.error = true, .next = ++it};
  }

  return {
      .code_point = decoded,
      .next = ++it,
  };
}

}  // namespace anitomy::detail::unicode::utf32
