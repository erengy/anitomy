#pragma once

namespace anitomy::detail {

[[nodiscard]] constexpr bool is_open_bracket(const char32_t ch) noexcept {
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

[[nodiscard]] constexpr bool is_close_bracket(const char32_t ch) noexcept {
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

[[nodiscard]] constexpr bool is_bracket(const char32_t ch) noexcept {
  return is_open_bracket(ch) || is_close_bracket(ch);
}

}  // namespace anitomy::detail
