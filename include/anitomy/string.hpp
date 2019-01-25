/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <string>

namespace anitomy {

using char_t = wchar_t;
using string_t = std::basic_string<char_t>;

bool IsAlphanumericChar(const char_t c);
bool IsHexadecimalChar(const char_t c);
bool IsLatinChar(const char_t c);
bool IsNumericChar(const char_t c);
bool IsAlphanumericString(const string_t& str);
bool IsHexadecimalString(const string_t& str);
bool IsMostlyLatinString(const string_t& str);
bool IsNumericString(const string_t& str);

bool IsInString(const string_t& str1, const string_t& str2);
bool IsStringEqualTo(const string_t& str1, const string_t& str2);

int StringToInt(const string_t& str);

void EraseString(string_t& str, const string_t& erase_this);
string_t StringToUpperCopy(string_t str);
void TrimString(string_t& str, const char_t trim_chars[] = L" ");

}  // namespace anitomy
