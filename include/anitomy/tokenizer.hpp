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

token_container_t Tokenize(const string_view_t filename, const Options& options);

void TokenizeByBrackets(string_view_t view, const Options& options, token_container_t& tokens);
void TokenizeByDelimiters(string_view_t view, const Options& options, const bool enclosed, token_container_t& tokens);

void ValidateTokens(token_container_t& tokens);

}  // namespace anitomy
