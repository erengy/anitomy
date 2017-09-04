/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <cwctype>
#include <functional>

#include "string.h"

namespace anitomy {

bool IsAlphanumericChar(const char_t c) {
  return (c >= L'0' && c <= L'9') ||
         (c >= L'A' && c <= L'Z') ||
         (c >= L'a' && c <= L'z');
}

bool IsHexadecimalChar(const char_t c) {
  return (c >= L'0' && c <= L'9') ||
         (c >= L'A' && c <= L'F') ||
         (c >= L'a' && c <= L'f');
}

bool IsLatinChar(const char_t c) {
  // We're just checking until the end of Latin Extended-B block, rather than
  // all the blocks that belong to the Latin script.
  return c <= L'\u024F';
}

bool IsNumericChar(const char_t c) {
  return c >= L'0' && c <= L'9';
}

bool IsAlphanumericString(const string_t& str) {
  return !str.empty() &&
         std::all_of(str.begin(), str.end(), IsAlphanumericChar);
}

bool IsHexadecimalString(const string_t& str) {
  return !str.empty() &&
         std::all_of(str.begin(), str.end(), IsHexadecimalChar);
}

bool IsMostlyLatinString(const string_t& str) {
  double length = str.empty() ? 1.0 : str.length();
  return std::count_if(str.begin(), str.end(), IsLatinChar) / length >= 0.5;
}

bool IsNumericString(const string_t& str) {
  return !str.empty() &&
         std::all_of(str.begin(), str.end(), IsNumericChar);
}

////////////////////////////////////////////////////////////////////////////////

inline wchar_t ToLower(const wchar_t c) {
  return (c >= L'a' && c <= L'z') ? c :
         (c >= L'A' && c <= L'Z') ? (c + (L'a' - L'A')) :
         static_cast<wchar_t>(std::towlower(c));
}

inline wchar_t ToUpper(const wchar_t c) {
  return (c >= L'A' && c <= L'Z') ? c :
         (c >= L'a' && c <= L'z') ? (c + (L'A' - L'a')) :
         static_cast<wchar_t>(std::towupper(c));
}

////////////////////////////////////////////////////////////////////////////////

bool IsInString(const string_t& str1, const string_t& str2) {
  return std::search(str1.begin(), str1.end(),
                     str2.begin(), str2.end()) != str1.end();
}

inline bool IsCharEqualTo(const char_t c1, const char_t c2) {
  return ToLower(c1) == ToLower(c2);
}

bool IsStringEqualTo(const string_t& str1, const string_t& str2) {
  return str1.size() == str2.size() &&
         std::equal(str1.begin(), str1.end(), str2.begin(), IsCharEqualTo);
}

////////////////////////////////////////////////////////////////////////////////

int StringToInt(const string_t& str) {
  return static_cast<int>(std::wcstol(str.c_str(), nullptr, 10));
}

////////////////////////////////////////////////////////////////////////////////

void EraseString(string_t& str, const string_t& erase_this) {
  if (erase_this.empty() || str.size() < erase_this.size())
    return;

  auto pos = str.find(erase_this);
  while (pos != string_t::npos) {
    str.erase(pos, erase_this.size());
    pos = str.find(erase_this);
  }
}

void StringToUpper(string_t& str) {
  std::transform(str.begin(), str.end(), str.begin(), ToUpper);
}

string_t StringToUpperCopy(string_t str) {
  StringToUpper(str);
  return str;
}

void TrimString(string_t& str, const char_t trim_chars[]) {
  if (str.empty())
    return;

  const auto pos_begin = str.find_first_not_of(trim_chars);
  const auto pos_end = str.find_last_not_of(trim_chars);

  if (pos_begin == string_t::npos || pos_end == string_t::npos) {
    str.clear();
    return;
  }

  str.erase(pos_end + 1, str.length() - pos_end + 1);
  str.erase(0, pos_begin);
}

}  // namespace anitomy
