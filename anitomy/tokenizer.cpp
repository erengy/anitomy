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

#include <map>

#include "string.h"
#include "tokenizer.h"

namespace anitomy {

Tokenizer::Tokenizer(const string_t& filename, token_container_t& tokens)
    : filename_(filename),
      tokens_(&tokens) {
}

bool Tokenizer::Tokenize() {
  tokens_->reserve(32);  // Usually there are no more than 20 tokens

  TokenizeByBrackets();

  return !tokens_->empty();
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::AddToken(TokenCategory category, bool enclosed,
                         const TokenRange& range) {
  tokens_->push_back(Token(category,
                           filename_.substr(range.offset, range.size),
                           enclosed));
}

void Tokenizer::TokenizeByBrackets() {
  static const string_t kOpeningBrackets = _TEXT("[({");
  static const string_t kClosingBrackets = _TEXT("])}");

  bool bracket_open = false;
  size_t last_bracket_index = 0;

  TokenRange range;

  for (size_t offset = 0; offset < filename_.size(); offset++) {
    const auto& brackets = bracket_open ? kClosingBrackets : kOpeningBrackets;
    size_t index = brackets.find(filename_.at(offset));

    // Character is a bracket
    if (index != string_t::npos) {
      // Check if it matches last open bracket
      if (bracket_open) {
        size_t bracket_index = index;
        if (bracket_index != last_bracket_index) {
          continue;
        }
      } else {
        last_bracket_index = index;
      }

      // Add unknown token
      if (range.offset < offset) {
        range.size = offset - range.offset;
        TokenizeByDelimiter(bracket_open, range);
      }
      // Add bracket
      AddToken(kBracket, true, TokenRange(offset, 1));
      bracket_open = !bracket_open;
      range.offset = offset + 1;

    // Character is not a bracket, and the loop reached the end
    } else if (offset == filename_.size() - 1) {
      // Add last unknown token
      range.size = offset - range.offset + 1;
      TokenizeByDelimiter(false, range);
    }
  }
}

void Tokenizer::TokenizeByDelimiter(bool enclosed, const TokenRange& range) {
  // Each token occasionally has a different delimiter, which is why we can't
  // analyze the whole thing in one go.
  char_t delimiter = GetDelimiter(range);

  // TODO: Better handle tokens with multiple delimiters
  if (!ValidateDelimiter(delimiter, enclosed, range)) {
    AddToken(kUnknown, enclosed, range);
    return;
  }

  TokenRange new_range(range.offset, 0);

  for (size_t offset = range.offset;
       offset < range.offset + range.size; offset++) {
    char_t character = filename_.at(offset);

    if (character == delimiter) {
      // Add new unknown token
      if (new_range.offset < offset) {
        new_range.size = offset - new_range.offset;
        AddToken(kUnknown, enclosed, new_range);
      }
      // Add delimiter
      AddToken(kDelimiter, enclosed, TokenRange(offset, 1));
      new_range.offset = offset + 1;
    } else if (offset == range.offset + range.size - 1) {
      // Add last unknown token
      new_range.size = offset - new_range.offset + 1;
      AddToken(kUnknown, enclosed, new_range);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

char_t Tokenizer::GetDelimiter(const TokenRange& range) {
  // Symbols are sorted by their precedence, in decreasing order. While the most
  // common delimiters are underscore, space and dot, we give comma the priority
  // to handle the case where words are separated by ", ". Besides, we'll be
  // trimming whitespace later on.
  static const string_t kDelimiterTable = _TEXT(",_ .-+;&|~");

  // Trim whitespace so that it doesn't interfere with our frequency analysis.
  // This proves useful for handling some edge cases, and it doesn't seem to
  // have any side effects.
  size_t offset = filename_.find_first_not_of(L' ', range.offset);
  size_t offset_end = offset == string_t::npos ? offset :
      filename_.find_last_not_of(L' ', range.offset + range.size - 1);
  if (offset == string_t::npos || offset_end < offset) {
    return L' ';  // There's nothing but whitespace
  }
  size_t size = offset_end - offset + 1;

  std::map<char_t, int> frequency;

  // Count all possible delimiters
  for (size_t i = offset; i < offset + size; i++) {
    char_t character = filename_.at(i);
    if (IsAlphanumericChar(character))
      continue;
    if (kDelimiterTable.find(character) == string_t::npos)
      continue;
    frequency[character] += 1;
  }

  char_t delimiter = L'\0';

  for (auto it = frequency.begin(); it != frequency.end(); ++it) {
    // Initialize delimiter at first iteration
    if (delimiter == L'\0') {
      delimiter = it->first;
      continue;
    }

    int character_distance =
        static_cast<int>(kDelimiterTable.find(it->first)) -
        static_cast<int>(kDelimiterTable.find(delimiter));
    // If the distance is negative, then the new delimiter has higher priority
    if (character_distance < 0) {
      delimiter = it->first;
      continue;
    }

    // Even if the new delimiter has lower priority, it may be much more common
    float frequency_ratio = static_cast<float>(it->second) /
                            static_cast<float>(frequency[delimiter]);
    // The constant value was chosen by trial and error. There should be room
    // for improvement.
    if (frequency_ratio / character_distance > 0.8f) {
      delimiter = it->first;
    }
  }

  return delimiter;
}

bool Tokenizer::ValidateDelimiter(char_t delimiter, bool enclosed,
                                  const TokenRange& range) {
  if (delimiter == L'\0')
    return false;

  // This prevents splitting some group names (e.g. "m.3.3.w") and keywords
  // (e.g. "H.264"). Ignoring " " yields better results in some edge cases.
  if (enclosed && delimiter != L' ') {
    size_t last_offset = range.offset;
    for (size_t offset = range.offset;
         offset < range.offset + range.size; offset++) {
      if (filename_.at(offset) == delimiter) {
        if (offset - last_offset == 1) {
          return false;  // Found a single-character token
        } else {
          last_offset = offset + 1;
        }
      }
    }
  }

  return true;
}

}  // namespace anitomy