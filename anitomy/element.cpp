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

#include <algorithm>

#include "element.h"

namespace anitomy {

size_t Elements::Count(ElementCategory category) const {
  return std::count_if(elements_.begin(), elements_.end(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

bool Elements::Empty(ElementCategory category) const {
  return Find(category) == elements_.end();
}

////////////////////////////////////////////////////////////////////////////////

string_t& Elements::operator[](ElementCategory category) {
  auto element = Find(category);

  if (element == elements_.end()) {
    elements_.push_back(std::make_pair(category, string_t()));
    return elements_.back().second;
  }

  return element->second;
}

std::vector<string_t> Elements::operator[](ElementCategory category) const {
  std::vector<string_t> elements;

  std::for_each(elements_.cbegin(), elements_.cend(),
      [&](const element_pair_t& element) {
        if (element.first == category)
          elements.push_back(element.second);
        });

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

void Elements::Add(ElementCategory category, const string_t& value) {
  if (!value.empty())
    elements_.push_back(std::make_pair(category, value));
}

void Elements::Clear() {
  elements_.clear();
}

////////////////////////////////////////////////////////////////////////////////

element_iterator_t Elements::Find(ElementCategory category) {
  return std::find_if(elements_.begin(), elements_.end(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

element_const_iterator_t Elements::Find(ElementCategory category) const {
  return std::find_if(elements_.cbegin(), elements_.cend(),
      [&](const element_pair_t& element) {
        return element.first == category;
      });
}

}  // namespace anitomy