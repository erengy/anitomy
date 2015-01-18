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

template<class Predicate>
token_iterator_t GetPreviousToken(token_container_t& tokens,
                                  token_iterator_t it,
                                  Predicate predicate) {
  if (it == tokens.begin())
    return tokens.end();

  do {
    --it;
  } while (it != tokens.begin() && !predicate(it));

  return it;
}

template<class Predicate>
token_iterator_t GetNextToken(token_container_t& tokens,
                              token_iterator_t it,
                              Predicate predicate) {
  do {
    ++it;
  } while (it != tokens.end() && !predicate(it));

  return it;
}

static bool IsTokenNotDelimiter(const token_iterator_t& it) {
  return it->category != kDelimiter;
}

static bool IsTokenValid(const token_iterator_t& it) {
  return it->category != kInvalid;
}

token_iterator_t GetPreviousNonDelimiterToken(token_container_t& tokens,
                                              token_iterator_t it) {
  return GetPreviousToken(tokens, it, IsTokenNotDelimiter);
}

token_iterator_t GetNextNonDelimiterToken(token_container_t& tokens,
                                          token_iterator_t it) {
  return GetNextToken(tokens, it, IsTokenNotDelimiter);
}

token_iterator_t GetPreviousValidToken(token_container_t& tokens,
                                       token_iterator_t it) {
  return GetPreviousToken(tokens, it, IsTokenValid);
}

token_iterator_t GetNextValidToken(token_container_t& tokens,
                                   token_iterator_t it) {
  return GetNextToken(tokens, it, IsTokenValid);
}

}  // namespace anitomy