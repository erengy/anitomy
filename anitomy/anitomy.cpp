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

#include "anitomy.h"
#include "keyword.h"
#include "parser.h"
#include "string.h"
#include "tokenizer.h"

namespace anitomy {

bool Anitomy::Parse(string_t filename) {
  elements_.clear();
  tokens_.clear();

  string_t extension;
  if (RemoveExtensionFromFilename(filename, extension))
    elements_.insert(kElementFileExtension, extension);

  if (filename.empty())
    return false;
  elements_.insert(kElementFileName, filename);

  std::vector<TokenRange> preidentified_tokens;
  keyword_manager.Peek(filename, elements_, preidentified_tokens);

  Tokenizer tokenizer(filename, tokens_, preidentified_tokens);
  if (!tokenizer.Tokenize())
    return false;

  Parser parser(elements_, tokens_);
  if (!parser.Parse())
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Anitomy::RemoveExtensionFromFilename(string_t& filename,
                                          string_t& extension) {
  const size_t position = filename.find_last_of(L'.');

  if (position == string_t::npos)
    return false;

  extension = filename.substr(position + 1);

  const size_t max_length = 4;
  if (extension.length() > max_length)
    return false;

  if (!IsAlphanumericString(extension))
    return false;

  // TODO: Add an option for this
  auto keyword = StringToUpperCopy(extension);
  if (!keyword_manager.Find(kElementFileExtension, keyword))
    return false;

  filename.resize(position);

  return true;
}

Elements& Anitomy::elements() {
  return elements_;
}

const token_container_t& Anitomy::tokens() const {
  return tokens_;
}

}  // namespace anitomy