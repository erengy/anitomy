/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <initializer_list>
#include <map>
#include <vector>

#include "element.h"
#include "string.h"

namespace anitomy {

struct TokenRange;

struct KeywordOptions {
  bool identifiable = true;
  bool searchable = true;
  bool valid = true;
};

struct Keyword {
  ElementCategory category;
  KeywordOptions options;
};

class KeywordManager {
public:
  KeywordManager();

  void Add(ElementCategory category, const KeywordOptions& options,
           const std::initializer_list<string_t>& keywords);

  bool Find(ElementCategory category, const string_t& str) const;
  bool Find(const string_t& str, ElementCategory& category, KeywordOptions& options) const;

  void Peek(const string_t& filename, const TokenRange& range, Elements& elements,
            std::vector<TokenRange>& preidentified_tokens) const;

  string_t Normalize(const string_t& str) const;

private:
  using keyword_container_t = std::map<string_t, Keyword>;

  keyword_container_t& GetKeywordContainer(ElementCategory category) const;

  keyword_container_t file_extensions_;
  keyword_container_t keys_;
};

extern KeywordManager keyword_manager;

}  // namespace anitomy
