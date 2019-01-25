/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

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
