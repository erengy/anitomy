/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

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

struct TokenRange {
  size_t offset = 0;
  size_t size = 0;
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

using token_container_t = std::vector<Token>;
using token_iterator_t = token_container_t::iterator;
using token_reverse_iterator_t = token_container_t::reverse_iterator;

token_iterator_t FindToken(token_iterator_t first, token_iterator_t last, unsigned int flags);
token_reverse_iterator_t FindToken(token_reverse_iterator_t first, token_reverse_iterator_t last, unsigned int flags);
token_iterator_t FindPreviousToken(token_container_t& tokens, token_iterator_t first, unsigned int flags);
token_iterator_t FindNextToken(token_container_t& tokens, token_iterator_t first, unsigned int flags);

}  // namespace anitomy
