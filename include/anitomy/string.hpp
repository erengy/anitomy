/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <string>
#include <string_view>

namespace anitomy {

using char_t = wchar_t;
using string_t = std::basic_string<char_t>;
using string_view_t = std::basic_string_view<char_t>;

}  // namespace anitomy
