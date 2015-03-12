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

#ifndef ANITOMY_TOKEN_H
#define ANITOMY_TOKEN_H

#include <vector>

#include "string.h"

namespace anitomy {

enum TokenCategory {
  kUnknown,
  kBracket,
  kDelimiter,
  kIdentifier,
  kInvalid
};

enum TokenFlag {
  kFlagNone,
  // Categories
  kFlagBracket     = 1 << 0,  kFlagNotBracket    = 1 << 1,
  kFlagDelimiter   = 1 << 2,  kFlagNotDelimiter  = 1 << 3,
  kFlagIdentifier  = 1 << 4,  kFlagNotIdentifier = 1 << 5,
  kFlagUnknown     = 1 << 6,  kFlagNotUnknown    = 1 << 7,
  kFlagValid       = 1 << 8,  kFlagNotValid      = 1 << 9,
  // Enclosed
  kFlagEnclosed    = 1 << 10, kFlagNotEnclosed   = 1 << 11,
  // Masks
  kFlagMaskCategories = kFlagBracket | kFlagNotBracket |
                        kFlagDelimiter | kFlagNotDelimiter |
                        kFlagIdentifier | kFlagNotIdentifier |
                        kFlagUnknown | kFlagNotUnknown |
                        kFlagValid | kFlagNotValid,
  kFlagMaskEnclosed = kFlagEnclosed | kFlagNotEnclosed,
};

class TokenRange {
public:
  TokenRange();
  TokenRange(size_t offset, size_t size);

  size_t offset;
  size_t size;
};

class Token {
public:
  Token();
  Token(TokenCategory category, const string_t& content, bool enclosed);

  bool operator==(const Token& token) const;

  TokenCategory category;
  string_t content;
  bool enclosed;
};

typedef std::vector<Token> token_container_t;
typedef token_container_t::iterator token_iterator_t;
typedef token_container_t::reverse_iterator token_reverse_iterator_t;

token_iterator_t FindToken(token_iterator_t first, token_iterator_t last, unsigned int flags);
token_reverse_iterator_t FindToken(token_reverse_iterator_t first, token_reverse_iterator_t last, unsigned int flags);
token_iterator_t FindPreviousToken(token_container_t& tokens, token_iterator_t first, unsigned int flags);
token_iterator_t FindNextToken(token_container_t& tokens, token_iterator_t first, unsigned int flags);

}  // namespace anitomy

#endif  // ANITOMY_TOKEN_H