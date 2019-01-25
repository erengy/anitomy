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

string_t Elements::get(ElementCategory category) const {
  auto element = find(category);
  return element != elements_.end() ? element->value : string_t();
}

std::vector<string_t> Elements::get_all(ElementCategory category) const {
  std::vector<string_t> elements;

  for (const auto& element : elements_)
    if (element.category == category)
      elements.push_back(element.value);

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
      [&](const Element& element) {
        return element.category == category;
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
    element->value = value;
  }
}

string_t& Elements::operator[](ElementCategory category) {
  auto element = find(category);

  if (element == elements_.end())
    element = elements_.insert(elements_.end(), {category, string_t()});

  return element->value;
}

////////////////////////////////////////////////////////////////////////////////

size_t Elements::count(ElementCategory category) const {
  return std::count_if(elements_.begin(), elements_.end(),
      [&](const Element& element) {
        return element.category == category;
      });
}

bool Elements::empty(ElementCategory category) const {
  return find(category) == elements_.end();
}

element_iterator_t Elements::find(ElementCategory category) {
  return std::find_if(elements_.begin(), elements_.end(),
      [&](const Element& element) {
        return element.category == category;
      });
}

element_const_iterator_t Elements::find(ElementCategory category) const {
  return std::find_if(elements_.cbegin(), elements_.cend(),
      [&](const Element& element) {
        return element.category == category;
      });
}

}  // namespace anitomy
