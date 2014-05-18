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

#include "token.h"

namespace anitomy {

TokenRange::TokenRange()
    : offset(0),
      size(0) {
}

TokenRange::TokenRange(size_t offset, size_t size)
    : offset(offset),
      size(size) {
}

Token::Token()
    : category(kUnknown),
      enclosed(false) {
}

Token::Token(TokenCategory category, const string_t& content, bool enclosed)
    : category(category),
      content(content),
      enclosed(enclosed) {
}

}  // namespace anitomy