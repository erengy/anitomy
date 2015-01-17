/*
** Anitomy
** Copyright (C) 2014-2015, Eren Okka
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

#ifndef ANITOMY_STRING_H
#define ANITOMY_STRING_H

#include <string>

namespace anitomy {

typedef wchar_t char_t;
typedef std::basic_string<char_t> string_t;

bool IsAlphanumericChar(const char_t c);
bool IsHexadecimalChar(const char_t c);
bool IsNumericChar(const char_t c);
bool IsAlphanumericString(const string_t& str);
bool IsHexadecimalString(const string_t& str);
bool IsNumericString(const string_t& str);

bool IsStringEqualTo(const string_t& str1, const string_t& str2);

int StringToInt(const string_t& str);

string_t StringToUpperCopy(string_t str);
void TrimString(string_t& str, const char_t trim_chars[] = L" ");

}  // namespace anitomy

#endif  // ANITOMY_STRING_H