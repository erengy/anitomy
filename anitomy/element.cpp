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

element_pair_t& Elements::operator[](size_t position) {
  return elements_[position];
}

const element_pair_t& Elements::operator[](size_t position) const {
  return elements_[position];
}

////////////////////////////////////////////////////////////////////////////////

string_t& Elements::get(ElementCategory category) {
  auto element = find(category);

  if (element == elements_.end())
    element = elements_.insert(elements_.end(),
                               std::make_pair(category, string_t()));

  return element->second;
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
    elements_.push_back(std::make_pair(category, value));
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