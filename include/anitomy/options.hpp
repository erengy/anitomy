/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <vector>

#include "string.h"

namespace anitomy {

struct Options {
  string_t allowed_delimiters = L" _.&+,|";
  std::vector<string_t> ignored_strings;

  bool parse_episode_number = true;
  bool parse_episode_title = true;
  bool parse_file_extension = true;
  bool parse_release_group = true;
};

}  // namespace anitomy
