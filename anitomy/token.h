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

#ifndef ANITOMY_TOKEN_H
#define ANITOMY_TOKEN_H

#include <vector>

#include "string.h"

namespace anitomy {

enum TokenCategory {
  kUnknown,
  kBracket,
  kDelimiter,
  kIdentifier
};

class TokenRange {
public:
  TokenRange();
  TokenRange(size_t offset, size_t size);

  size_t offset;
  size_t size;
};

bool RangesOverlap(const TokenRange& r1, const TokenRange& r2);

class Token {
public:
  Token();
  Token(TokenCategory category, const string_t& content, bool enclosed);

  TokenCategory category;
  string_t content;
  bool enclosed;
};

typedef std::vector<Token> token_container_t;
typedef token_container_t::iterator token_iterator_t;

}  // namespace anitomy

#endif  // ANITOMY_TOKEN_H