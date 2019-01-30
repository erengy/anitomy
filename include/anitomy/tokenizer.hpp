/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <anitomy/options.hpp>
#include <anitomy/string.hpp>
#include <anitomy/token.hpp>

namespace anitomy {

Tokens Tokenize(const string_view_t filename, const Options& options);

void TokenizeByBrackets(string_view_t view, const Options& options, Tokens& tokens);
void TokenizeByDelimiters(string_view_t view, const Options& options, const bool enclosed, Tokens& tokens);

void ValidateTokens(Tokens& tokens);

}  // namespace anitomy
