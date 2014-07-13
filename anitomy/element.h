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

#ifndef ANITOMY_ELEMENT_H
#define ANITOMY_ELEMENT_H

#include <vector>

#include "string.h"

namespace anitomy {

enum ElementCategory {
  kElementIterateFirst,
  kElementAnimeSeason = kElementIterateFirst,
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
  kElementIterateLast
};

typedef std::pair<ElementCategory, string_t> element_pair_t;
typedef std::vector<element_pair_t> element_container_t;

typedef element_container_t::iterator element_iterator_t;
typedef element_container_t::const_iterator element_const_iterator_t;

class Elements {
public:
  size_t Count(ElementCategory category) const;
  bool Empty(ElementCategory category) const;

  string_t& operator[](ElementCategory category);
  std::vector<string_t> operator[](ElementCategory category) const;

  void Add(ElementCategory category, const string_t& value);
  void Clear();

private:
  element_iterator_t Find(ElementCategory category);
  element_const_iterator_t Find(ElementCategory category) const;

  element_container_t elements_;
};

}  // namespace anitomy

#endif  // ANITOMY_ELEMENT_H