/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <initializer_list>
#include <map>
#include <vector>

#include <anitomy/element.hpp>
#include <anitomy/string.hpp>

namespace anitomy {

struct KeywordOptions {
  bool identifiable = true;
  bool searchable = true;
  bool valid = true;
};

struct Keyword {
  ElementType type;
  KeywordOptions options;
};

class KeywordManager {
public:
  KeywordManager();

  void Add(ElementType type, const KeywordOptions& options,
           const std::initializer_list<string_t>& keywords);

  bool Find(ElementType type, const string_t& str) const;
  bool Find(const string_t& str, ElementType& type, KeywordOptions& options) const;

  string_t Normalize(const string_t& str) const;

private:
  using keyword_container_t = std::map<string_t, Keyword>;

  keyword_container_t& GetKeywordContainer(ElementType type) const;

  keyword_container_t file_extensions_;
  keyword_container_t keys_;
};

extern KeywordManager keyword_manager;

}  // namespace anitomy
