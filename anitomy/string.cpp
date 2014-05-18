/*
** Anitomy
** Copyright (C) 2014, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "string.h"

namespace anitomy {

bool IsAlphanumericChar(const char_t c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

bool IsHexadecimalChar(const char_t c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

bool IsNumericChar(const char_t c) {
  return c >= '0' && c <= '9';
}

bool IsHexadecimalString(const string_t& str) {
  return !str.empty() &&
         std::all_of(str.begin(), str.end(), IsHexadecimalChar);
}

bool IsNumericString(const string_t& str) {
  return !str.empty() &&
         std::all_of(str.begin(), str.end(), IsNumericChar);
}

////////////////////////////////////////////////////////////////////////////////

inline char_t ToLowerA(const char_t c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

inline bool IsCharEqualTo(const char_t c1, const char_t c2) {
  return tolower(c1) == tolower(c2);
}

inline bool IsCharEqualToA(const char_t c1, const char_t c2) {
  return ToLowerA(c1) == ToLowerA(c2);
}

bool IsStringEqualTo(const string_t& str1, const string_t& str2) {
  return str1.size() == str2.size() &&
         std::equal(str1.begin(), str1.end(), str2.begin(), IsCharEqualTo);
}

bool IsStringEqualToA(const string_t& str1, const string_t& str2) {
  return str1.size() == str2.size() &&
         std::equal(str1.begin(), str1.end(), str2.begin(), IsCharEqualToA);
}

////////////////////////////////////////////////////////////////////////////////

int StringToInt(const std::string& str) {
  return static_cast<int>(std::strtoul(str.c_str(), nullptr, 10));
}

int StringToInt(const std::wstring& str) {
  return static_cast<int>(std::wcstol(str.c_str(), nullptr, 10));
}

////////////////////////////////////////////////////////////////////////////////

void StringToUpper(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void StringToUpper(std::wstring& str) {
  std::transform(str.begin(), str.end(), str.begin(), towupper);
}

string_t StringToUpperCopy(string_t str) {
  StringToUpper(str);
  return str;
}

void TrimString(string_t& str, const char_t trim_chars[]) {
  if (str.empty())
    return;

  const string_t::size_type pos_begin = str.find_first_not_of(trim_chars);
  const string_t::size_type pos_end = str.find_last_not_of(trim_chars);

  if (pos_begin == string_t::npos || pos_end == string_t::npos) {
    str.clear();
    return;
  }

  str.erase(pos_end + 1, str.length() - pos_end + 1);
  str.erase(0, pos_begin);
}

}  // namespace anitomy