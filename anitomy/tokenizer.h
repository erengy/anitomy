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

#ifndef ANITOMY_TOKENIZER_H
#define ANITOMY_TOKENIZER_H

#include "string.h"
#include "token.h"

namespace anitomy {

class Tokenizer {
public:
  Tokenizer(const string_t& filename, token_container_t& tokens);

  bool Tokenize();

private:
  void AddToken(TokenCategory category, bool enclosed, const TokenRange& range);
  void TokenizeByBrackets();
  void TokenizeByDelimiter(bool enclosed, const TokenRange& range);

  char_t GetDelimiter(TokenRange range) const;
  bool ValidateDelimiter(const char_t delimiter, bool enclosed, const TokenRange& range) const;

  const string_t& filename_;
  token_container_t& tokens_;
};

}  // namespace anitomy

#endif  // ANITOMY_TOKENIZER_H