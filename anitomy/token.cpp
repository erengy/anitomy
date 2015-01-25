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

#include <algorithm>

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

////////////////////////////////////////////////////////////////////////////////

Token::Token()
    : category(kUnknown),
      enclosed(false) {
}

Token::Token(TokenCategory category, const string_t& content, bool enclosed)
    : category(category),
      content(content),
      enclosed(enclosed) {
}

////////////////////////////////////////////////////////////////////////////////

static bool CheckTokenFlags(const Token& token, unsigned int flags) {
  auto check_flag = [&flags](unsigned int flag) {
    return (flags & flag) == flag;
  };

  if (flags & kFlagMaskEnclosed) {
    bool success = check_flag(kFlagEnclosed) ? token.enclosed : !token.enclosed;
    if (!success)
      return false;
  }

  if (flags & kFlagMaskCategories) {
    bool success = false;
    auto check_category = [&](TokenFlag fe, TokenFlag fn, TokenCategory c) {
      if (!success)
        success = check_flag(fe) ? token.category == c :
                  check_flag(fn) ? token.category != c : false;
    };
    check_category(kFlagBracket, kFlagNotBracket, kBracket);
    check_category(kFlagDelimiter, kFlagNotDelimiter, kDelimiter);
    check_category(kFlagIdentifier, kFlagNotIdentifier, kIdentifier);
    check_category(kFlagUnknown, kFlagNotUnknown, kUnknown);
    check_category(kFlagNotValid, kFlagValid, kInvalid);
    if (!success)
      return false;
  }

  return true;
}

template<class iterator_t>
static iterator_t FindTokenBase(iterator_t first, iterator_t last,
                                unsigned int flags) {
  return std::find_if(first, last, [&](const Token& token) {
        return CheckTokenFlags(token, flags);
      });
}

token_iterator_t FindToken(token_iterator_t first, token_iterator_t last,
                           unsigned int flags) {
  return FindTokenBase(first, last, flags);
}

token_reverse_iterator_t FindToken(token_reverse_iterator_t first,
                                   token_reverse_iterator_t last,
                                   unsigned int flags) {
  return FindTokenBase(first, last, flags);
}

token_iterator_t FindPreviousToken(token_container_t& tokens,
                                   token_iterator_t first,
                                   unsigned int flags) {
  auto it = FindToken(std::reverse_iterator<token_iterator_t>(first),
                       tokens.rend(), flags);
  return it == tokens.rend() ? tokens.end() : (++it).base();
}

token_iterator_t FindNextToken(token_container_t& tokens,
                               token_iterator_t first,
                               unsigned int flags) {
  return FindToken(++first, tokens.end(), flags);
}

}  // namespace anitomy