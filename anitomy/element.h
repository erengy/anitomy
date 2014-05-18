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

#include "string.h"

namespace anitomy {

typedef string_t element_container_t;  // TODO: use vector

class Elements {
public:
  void Clear();

  string_t filename;

  string_t anime_season;
  string_t anime_title;
  string_t anime_year;

  element_container_t episode_number;
  string_t episode_title;

  string_t release_group;
  string_t release_version;

  element_container_t audio;
  element_container_t extras;
  element_container_t video;
  string_t checksum;
  string_t resolution;
};

}  // namespace anitomy

#endif  // ANITOMY_ELEMENT_H