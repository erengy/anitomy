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

#ifndef ANITOMY_ELEMENT_H
#define ANITOMY_ELEMENT_H

#include <vector>

#include "string.h"

namespace anitomy {

enum ElementCategory {
  kElementIterateFirst,
  kElementAnimeSeason = kElementIterateFirst,
  kElementAnimeSeasonPrefix,
  kElementAnimeTitle,
  kElementAnimeType,
  kElementAnimeYear,
  kElementAudioTerm,
  kElementDeviceCompatibility,
  kElementEpisodeNumber,
  kElementEpisodePrefix,
  kElementEpisodeTitle,
  kElementFileChecksum,
  kElementFileExtension,
  kElementFileName,
  kElementLanguage,
  kElementOther,
  kElementReleaseGroup,
  kElementReleaseInformation,
  kElementReleaseVersion,
  kElementSource,
  kElementSubtitles,
  kElementVideoResolution,
  kElementVideoTerm,
  kElementIterateLast,
  kElementUnknown = kElementIterateLast
};

typedef std::pair<ElementCategory, string_t> element_pair_t;
typedef std::vector<element_pair_t> element_container_t;

typedef element_container_t::iterator element_iterator_t;
typedef element_container_t::const_iterator element_const_iterator_t;

class Elements {
public:
  // Capacity
  bool empty() const;
  size_t size() const;

  // Iterators
  element_iterator_t begin();
  element_const_iterator_t begin() const;
  element_const_iterator_t cbegin() const;
  element_iterator_t end();
  element_const_iterator_t end() const;
  element_const_iterator_t cend() const;

  // Element access
  element_pair_t& at(size_t position);
  const element_pair_t& at(size_t position) const;
  element_pair_t& operator[](size_t position);
  const element_pair_t& operator[](size_t position) const;

  // Value access
  string_t& get(ElementCategory category);
  std::vector<string_t> get_all(ElementCategory category) const;

  // Modifiers
  void clear();
  void insert(ElementCategory category, const string_t& value);

  // Lookup
  size_t count(ElementCategory category) const;
  bool empty(ElementCategory category) const;
  element_iterator_t find(ElementCategory category);
  element_const_iterator_t find(ElementCategory category) const;

private:
  element_container_t elements_;
};

}  // namespace anitomy

#endif  // ANITOMY_ELEMENT_H