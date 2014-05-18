/*
** Anitomy
** Copyright (C) 2014, Eren Okka
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

#include <map>
#include <unordered_set>

#include "string.h"

namespace anitomy {

enum KeywordCategory {
  kKeywordAudio,
  kKeywordVideo,
  kKeywordExtra,
  kKeywordExtraUnsafe,
  kKeywordVersion,
  kKeywordValidExtension,
  kKeywordEpisodePrefix,
  kKeywordGroup
};

class KeywordList {
public:
  KeywordList();
  ~KeywordList() {}

  void Add(const string_t& str);
  bool Find(const string_t& str) const;

private:
  std::unordered_set<string_t> keys_;
  size_t max_length_;
  size_t min_length_;
};

class KeywordManager {
public:
  KeywordManager();
  ~KeywordManager() {}

  void Add(KeywordCategory category, const string_t& input);
  bool Find(KeywordCategory category, const string_t& str);

private:
  std::map<KeywordCategory, KeywordList> keyword_lists_;
};

extern KeywordManager keyword_manager;

}  // namespace anitomy

#endif  // ANITOMY_KEYWORD_H