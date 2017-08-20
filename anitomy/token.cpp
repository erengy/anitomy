/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include "token.h"

namespace anitomy {

Token::Token()
    : category(kUnknown),
      enclosed(false) {
}

Token::Token(TokenCategory category, const string_t& content, bool enclosed)
    : category(category),
      content(content),
      enclosed(enclosed) {
}

bool Token::operator==(const Token& token) const {
  return category == token.category &&
         content == token.content &&
         enclosed == token.enclosed;
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
