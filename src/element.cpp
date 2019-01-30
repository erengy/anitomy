/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include <anitomy/element.hpp>

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

Element& Elements::at(size_t position) {
  return elements_.at(position);
}

const Element& Elements::at(size_t position) const {
  return elements_.at(position);
}

////////////////////////////////////////////////////////////////////////////////

string_t Elements::get(ElementType type) const {
  auto element = find(type);
  return element != elements_.end() ? element->value : string_t();
}

std::vector<string_t> Elements::get_all(ElementType type) const {
  std::vector<string_t> elements;

  for (const auto& element : elements_)
    if (element.type == type)
      elements.push_back(element.value);

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

void Elements::clear() {
  elements_.clear();
}

void Elements::insert(ElementType type, const string_t& value) {
  if (!value.empty())
    elements_.push_back({type, value});
}

void Elements::erase(ElementType type) {
  auto iterator = std::remove_if(elements_.begin(), elements_.end(),
      [&](const Element& element) {
        return element.type == type;
      });
  elements_.erase(iterator, elements_.end());
}

element_iterator_t Elements::erase(element_iterator_t iterator) {
  return elements_.erase(iterator);
}

void Elements::set(ElementType type, const string_t& value) {
  auto element = find(type);

  if (element == elements_.end()) {
    elements_.push_back({type, value});
  } else {
    element->value = value;
  }
}

string_t& Elements::operator[](ElementType type) {
  auto element = find(type);

  if (element == elements_.end())
    element = elements_.insert(elements_.end(), {type, string_t()});

  return element->value;
}

////////////////////////////////////////////////////////////////////////////////

size_t Elements::count(ElementType type) const {
  return std::count_if(elements_.begin(), elements_.end(),
      [&](const Element& element) {
        return element.type == type;
      });
}

bool Elements::empty(ElementType type) const {
  return find(type) == elements_.end();
}

element_iterator_t Elements::find(ElementType type) {
  return std::find_if(elements_.begin(), elements_.end(),
      [&](const Element& element) {
        return element.type == type;
      });
}

element_const_iterator_t Elements::find(ElementType type) const {
  return std::find_if(elements_.cbegin(), elements_.cend(),
      [&](const Element& element) {
        return element.type == type;
      });
}

}  // namespace anitomy
