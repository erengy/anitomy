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

class Anitomy {
public:
  bool Parse(string_t filename);

  Elements& elements();
  Options& options();
  const token_container_t& tokens() const;

private:
  bool RemoveExtensionFromFilename(string_t& filename, string_t& extension) const;
  void RemoveIgnoredStrings(string_t& filename) const;

  Elements elements_;
  Options options_;
  token_container_t tokens_;
};

}  // namespace anitomy
