/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include "element.h"

namespace anitomy {

bool Elements::empty() const {
  return elements_.empty();
}

size_t Elements::size() const {
  return elements_.size();
}

////////////////////////////////////////////////////////////////////////////////

element_iterator_t Elements::begin() {
  return elements_.begin();
}

element_const_iterator_t Elements::begin() const {
  return elements_.begin();
}

element_const_iterator_t Elements::cbegin() const {
  return elements_.begin();
}

element_iterator_t Elements::end() {
  return elements_.end();
}

element_const_iterator_t Elements::end() const {
  return elements_.end();
}

element_const_iterator_t Elements::cend() const {
  return elements_.end();
}

////////////////////////////////////////////////////////////////////////////////

element_pair_t& Elements::at(size_t position) {
  return elements_.at(position);
}

const element_pair_t& Elements::at(size_t position) const {
  return elements_.at(position);
}

////////////////////////////////////////////////////////////////////////////////

string_t Elements::get(ElementCategory category) const {
  auto element = find(category);
  return element != elements_.end() ? element->second : string_t();
}

std::vector<string_t> Elements::get_all(ElementCategory category) const {
  std::vector<string_t> elements;

  for (const auto& element : elements_)
    if (element.first == category)
      elements.push_back(element.second);

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

void Elements::clear() {
  elements_.clear();
}

void Elements::insert(ElementCategory category, const string_t& value) {
  if (!value.empty())
    elements_.push_back({category, value});
}

void Elements::erase(ElementCategory category) {
  auto iterator = std::remove_if(elements_.begin(), elements_.end(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
  elements_.erase(iterator, elements_.end());
}

element_iterator_t Elements::erase(element_iterator_t iterator) {
  return elements_.erase(iterator);
}

void Elements::set(ElementCategory category, const string_t& value) {
  auto element = find(category);

  if (element == elements_.end()) {
    elements_.push_back({category, value});
  } else {
    element->second = value;
  }
}

string_t& Elements::operator[](ElementCategory category) {
  auto element = find(category);

  if (element == elements_.end())
    element = elements_.insert(elements_.end(), {category, string_t()});

  return element->second;
}

////////////////////////////////////////////////////////////////////////////////

size_t Elements::count(ElementCategory category) const {
  return std::count_if(elements_.begin(), elements_.end(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

bool Elements::empty(ElementCategory category) const {
  return find(category) == elements_.end();
}

element_iterator_t Elements::find(ElementCategory category) {
  return std::find_if(elements_.begin(), elements_.end(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

element_const_iterator_t Elements::find(ElementCategory category) const {
  return std::find_if(elements_.cbegin(), elements_.cend(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

}  // namespace anitomy
