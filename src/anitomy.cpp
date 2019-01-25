/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

  if (options_.parse_file_extension) {
    string_t extension;
    if (RemoveExtensionFromFilename(filename, extension))
      elements_.insert(kElementFileExtension, extension);
  }

  if (!options_.ignored_strings.empty())
    RemoveIgnoredStrings(filename);

  if (filename.empty())
    return false;
  elements_.insert(kElementFileName, filename);

  Tokenizer tokenizer(filename, elements_, options_, tokens_);
  if (!tokenizer.Tokenize())
    return false;

  Parser parser(elements_, options_, tokens_);
  if (!parser.Parse())
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Anitomy::RemoveExtensionFromFilename(string_t& filename,
                                          string_t& extension) const {
  const size_t position = filename.find_last_of(L'.');

  if (position == string_t::npos)
    return false;

  extension = filename.substr(position + 1);

  const size_t max_length = 4;
  if (extension.length() > max_length)
    return false;

  if (!IsAlphanumericString(extension))
    return false;

  auto keyword = keyword_manager.Normalize(extension);
  if (!keyword_manager.Find(kElementFileExtension, keyword))
    return false;

  filename.resize(position);

  return true;
}

void Anitomy::RemoveIgnoredStrings(string_t& filename) const {
  for (const auto& str : options_.ignored_strings) {
    EraseString(filename, str);
  }
}

////////////////////////////////////////////////////////////////////////////////

Elements& Anitomy::elements() {
  return elements_;
}

Options& Anitomy::options() {
  return options_;
}

const token_container_t& Anitomy::tokens() const {
  return tokens_;
}

}  // namespace anitomy
