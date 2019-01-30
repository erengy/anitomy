/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <anitomy/element.hpp>
#include <anitomy/options.hpp>
#include <anitomy/string.hpp>
#include <anitomy/token.hpp>

namespace anitomy {

class Tokenizer {
public:
  Tokenizer(const Options& options, token_container_t& tokens);

  Tokenizer(const Tokenizer&) = delete;
  Tokenizer& operator=(const Tokenizer&) = delete;

  bool Tokenize(const string_view_t filename);

private:
  void TokenizeByBrackets(string_view_t view);
  void TokenizeByDelimiters(bool enclosed, string_view_t view);

  void ValidateDelimiterTokens();

  const Options& options_;
  token_container_t& tokens_;
};

}  // namespace anitomy
