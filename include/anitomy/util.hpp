#pragma once

#include <charconv>
#include <fstream>
#include <string_view>
#include <unordered_map>

namespace anitomy::detail {

inline std::string_view from_ordinal_number(std::string_view input) {
  static const std::unordered_map<std::string_view, std::string_view> table{
      // clang-format off
      {"1st", "1"}, {"First",   "1"},
      {"2nd", "2"}, {"Second",  "2"},
      {"3rd", "3"}, {"Third",   "3"},
      {"4th", "4"}, {"Fourth",  "4"},
      {"5th", "5"}, {"Fifth",   "5"},
      {"6th", "6"}, {"Sixth",   "6"},
      {"7th", "7"}, {"Seventh", "7"},
      {"8th", "8"}, {"Eighth",  "8"},
      {"9th", "9"}, {"Ninth",   "9"},
      // clang-format on
  };

  auto it = table.find(input);
  return it != table.end() ? it->second : std::string_view{};
}

inline std::string_view from_roman_number(std::string_view input) {
  static const std::unordered_map<std::string_view, std::string_view> table{
      // clang-format off
      {"II",  "2"},
      {"III", "3"},
      {"IV",  "4"},
      // clang-format on
  };

  auto it = table.find(input);
  return it != table.end() ? it->second : std::string_view{};
}

constexpr bool is_alpha(const char ch) noexcept {
  return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z');
}

constexpr bool is_digit(const char ch) noexcept {
  return '0' <= ch && ch <= '9';
}

constexpr bool is_xdigit(const char ch) noexcept {
  return ('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F') || ('a' <= ch && ch <= 'f');
}

constexpr int to_int(const std::string_view str) noexcept {
  int value{0};
  std::from_chars(str.data(), str.data() + str.size(), value, 10);
  return value;
}

template <typename Char>
constexpr Char to_lower(const Char ch) noexcept {
  return ('A' <= ch && ch <= 'Z') ? ch + ('a' - 'A') : ch;
}

inline bool read_file(const std::string& path, std::string& output) {
  std::ifstream file{path, std::ios::in | std::ios::binary | std::ios::ate};

  if (!file) return false;

  output.resize(static_cast<size_t>(file.tellg()));
  file.seekg(0, std::ios::beg);
  file.read(output.data(), output.size());

  return file.good();
}

}  // namespace anitomy::detail
