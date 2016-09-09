/*
** Copyright (c) 2014-2016, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
  kElementEpisodeNumberAlt,
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
  kElementVolumeNumber,
  kElementVolumePrefix,
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

  // Value access
  string_t get(ElementCategory category) const;
  std::vector<string_t> get_all(ElementCategory category) const;

  // Modifiers
  void clear();
  void insert(ElementCategory category, const string_t& value);
  void erase(ElementCategory category);
  element_iterator_t erase(element_iterator_t iterator);
  void set(ElementCategory category, const string_t& value);
  string_t& operator[](ElementCategory category);

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