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

#include "anitomy.h"
#include "parser.h"
#include "string.h"
#include "tokenizer.h"

namespace anitomy {

bool Anitomy::Parse(const string_t& filename) {
  data_.Clear();
  data_.filename = filename;
  tokens_.clear();

  RemoveExtensionFromFilename(data_.filename);

  if (data_.filename.empty())
    return false;

  Tokenizer tokenizer(data_.filename, tokens_);
  if (!tokenizer.Tokenize())
    return false;

  Parser parser(data_, tokens_);
  if (!parser.Parse())
    return false;

  return true;
}

void Anitomy::RemoveExtensionFromFilename(string_t& filename) {
  size_t position = filename.find_last_of(L".");

  if (position == string_t::npos)
    return;

  string_t extension = filename.substr(position + 1);

  const size_t max_length = 4;
  if (extension.length() > max_length)
    return;

  if (!IsAlphanumericString(extension))
    return;

  filename.resize(position);
}

const Elements& Anitomy::elements() const {
  return data_;
}

const token_container_t& Anitomy::tokens() const {
  return tokens_;
}

}  // namespace anitomy