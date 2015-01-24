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

#ifndef ANITOMY_TOKENIZER_H
#define ANITOMY_TOKENIZER_H

#include "element.h"
#include "options.h"
#include "string.h"
#include "token.h"

namespace anitomy {

class Tokenizer {
public:
  Tokenizer(const string_t& filename, Elements& elements,
            const Options& options, token_container_t& tokens);

  Tokenizer(const Tokenizer&) = delete;
  Tokenizer& operator=(const Tokenizer&) = delete;

  bool Tokenize();

private:
  void AddToken(TokenCategory category, bool enclosed, const TokenRange& range);
  void TokenizeByBrackets();
  void TokenizeByPreidentified(bool enclosed, const TokenRange& range);
  void TokenizeByDelimiters(bool enclosed, const TokenRange& range);

  string_t GetDelimiters(const TokenRange& range) const;
  void ValidateDelimiterTokens();

  Elements& elements_;
  const string_t& filename_;
  const Options& options_;
  token_container_t& tokens_;
};

}  // namespace anitomy

#endif  // ANITOMY_TOKENIZER_H