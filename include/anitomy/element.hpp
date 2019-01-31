/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <vector>

#include <anitomy/string.hpp>

namespace anitomy {

enum class ElementType {
  AnimeSeason,
  AnimeSeasonPrefix,
  AnimeTitle,
  AnimeType,
  AnimeYear,
  AudioTerm,
  DeviceCompatibility,
  EpisodeNumber,
  EpisodeNumberAlt,
  EpisodePrefix,
  EpisodeTitle,
  FileChecksum,
  FileExtension,
  FileName,
  Language,
  Other,
  ReleaseGroup,
  ReleaseInformation,
  ReleaseVersion,
  Source,
  Subtitles,
  Unknown,
  VideoResolution,
  VideoTerm,
  VolumeNumber,
  VolumePrefix,
};

struct Element {
  Element(ElementType type, string_view_t value)
      : type{type}, value{string_t{value}} {}

  ElementType type;
  string_t value;
};

class Elements {
public:
  using container_t = std::vector<Element>;
  using iterator_t = container_t::iterator;
  using const_iterator_t = container_t::const_iterator;

  // Capacity
  bool empty() const;
  size_t size() const;

  // Iterators
  iterator_t begin();
  const_iterator_t begin() const;
  const_iterator_t cbegin() const;
  iterator_t end();
  const_iterator_t end() const;
  const_iterator_t cend() const;

  // Element access
  Element& at(size_t position);
  const Element& at(size_t position) const;

  // Value access
  string_t get(ElementType type) const;
  std::vector<string_t> get_all(ElementType type) const;

  // Modifiers
  void clear();
  void insert(ElementType type, const string_t& value);
  void erase(ElementType type);
  iterator_t erase(iterator_t iterator);
  void set(ElementType type, const string_t& value);
  string_t& operator[](ElementType type);

  // Lookup
  size_t count(ElementType type) const;
  bool empty(ElementType type) const;
  iterator_t find(ElementType type);
  const_iterator_t find(ElementType type) const;

private:
  container_t elements_;
};

}  // namespace anitomy
