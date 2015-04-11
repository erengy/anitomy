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

#ifndef ANITOMY_KEYWORD_H
#define ANITOMY_KEYWORD_H

#include <initializer_list>
#include <map>
#include <vector>

#include "element.h"
#include "string.h"

namespace anitomy {

class TokenRange;

class KeywordOptions {
public:
  KeywordOptions() {}
  KeywordOptions(bool identifiable, bool searchable, bool valid);

  bool identifiable = true;
  bool searchable = true;
  bool valid = true;
};

class Keyword {
public:
  Keyword(ElementCategory category, const KeywordOptions& options);

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
  typedef std::map<string_t, Keyword> keyword_container_t;

  keyword_container_t& GetKeywordContainer(ElementCategory category) const;

  keyword_container_t file_extensions_;
  keyword_container_t keys_;
};

extern KeywordManager keyword_manager;

}  // namespace anitomy

#endif  // ANITOMY_KEYWORD_H