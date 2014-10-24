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
      tokens_(tokens) {
}

bool Tokenizer::Tokenize() {
  tokens_.reserve(32);  // Usually there are no more than 20 tokens

  TokenizeByBrackets();

  return !tokens_.empty();
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::AddToken(TokenCategory category, bool enclosed,
                         const TokenRange& range) {
  tokens_.push_back(Token(category,
                          filename_.substr(range.offset, range.size),
                          enclosed));
}

void Tokenizer::TokenizeByBrackets() {
  static const std::vector<std::pair<char_t, char_t>> brackets{
      {L'(', L')'},  // U+0028-U+0029 Parenthesis
      {L'[', L']'},  // U+005B-U+005D Square bracket
      {L'{', L'}'},  // U+007B-U+007D Curly bracket
      {L'\u300C', L'\u300D'},  // Corner bracket
      {L'\u300E', L'\u300F'},  // White corner bracket
      {L'\u3010', L'\u3011'},  // Black lenticular bracket
      {L'\uFF08', L'\uFF09'},  // Fullwidth parenthesis
  };

  bool is_bracket_open = false;
  char_t matching_bracket = L'\0';

  auto char_begin = filename_.begin();
  const auto char_end = filename_.end();

  // This is basically std::find_first_of() customized to our needs
  auto find_first_bracket = [&]() -> string_t::const_iterator {
    for (auto it = char_begin; it != char_end; ++it) {
      for (const auto& bracket_pair : brackets) {
        if (*it == bracket_pair.first) {
          matching_bracket = bracket_pair.second;
          return it;
        }
      }
    }
    return char_end;
  };

  auto current_char = char_begin;

  while (current_char != char_end && char_begin != char_end) {
    if (!is_bracket_open) {
      current_char = find_first_bracket();
    } else {
      // Looking for the matching bracket allows us to better handle some rare
      // cases with nested brackets.
      current_char = std::find(char_begin, char_end, matching_bracket);
    }

    const TokenRange range(std::distance(filename_.begin(), char_begin),
                           std::distance(char_begin, current_char));

    if (range.size > 0)  // Found unknown token
      TokenizeByDelimiter(is_bracket_open, range);

    if (current_char != char_end) {  // Found bracket
      AddToken(kBracket, true, TokenRange(range.offset + range.size, 1));
      is_bracket_open = !is_bracket_open;
      char_begin = ++current_char;
    }
  }
}

void Tokenizer::TokenizeByDelimiter(bool enclosed, const TokenRange& range) {
  // Each group occasionally has a different delimiter, which is why we can't
  // analyze the whole filename in one go.
  const char_t delimiter = GetDelimiter(range);

  // TODO: Better handle groups with multiple delimiters
  if (!ValidateDelimiter(delimiter, enclosed, range)) {
    AddToken(kUnknown, enclosed, range);
    return;
  }

  auto char_begin = filename_.begin() + range.offset;
  const auto char_end = char_begin + range.size;
  auto current_char = char_begin;

  while (current_char != char_end) {
    current_char = std::find(char_begin, char_end, delimiter);

    const TokenRange sub_range(std::distance(filename_.begin(), char_begin),
                               std::distance(char_begin, current_char));

    if (sub_range.size > 0)  // Found unknown token
      AddToken(kUnknown, enclosed, sub_range);

    if (current_char != char_end) {  // Found delimiter
      AddToken(kDelimiter, enclosed,
               TokenRange(sub_range.offset + sub_range.size, 1));
      char_begin = ++current_char;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

static bool TrimWhitespace(const string_t& str, TokenRange& range) {
  const char_t whitespace = L' ';

  size_t offset_end = range.offset + range.size - 1;

  range.offset = str.find_first_not_of(whitespace, range.offset);

  if (range.offset != string_t::npos) {
    offset_end = str.find_last_not_of(whitespace, offset_end);

    if (offset_end >= range.offset) {
      range.size = offset_end - range.offset + 1;
      return true;
    }
  }

  return false;  // There's nothing but whitespace
}

char_t Tokenizer::GetDelimiter(TokenRange range) const {
  // Symbols are sorted by their precedence, in decreasing order. While the most
  // common delimiters are underscore, space and dot, we give comma the priority
  // to handle the case where words are separated by ", ". Besides, we'll be
  // trimming whitespace later on.
  static const string_t kDelimiterTable = L",_ .-+;&|~";

  // Trim whitespace so that it doesn't interfere with our frequency analysis.
  // This proves useful for handling some edge cases, and it doesn't seem to
  // have any side effects.
  if (!TrimWhitespace(filename_, range))
    return L' ';

  static std::map<char_t, size_t> frequency;

  if (frequency.empty()) {
    // Initialize frequency map
    for (const auto& character : kDelimiterTable) {
      frequency.insert(std::make_pair(character, 0));
    }
  } else {
    // Reset frequency map
    for (auto& pair : frequency) {
      pair.second = 0;
    }
  }

  // Count all possible delimiters
  for (size_t i = range.offset; i < range.offset + range.size; i++) {
    const char_t character = filename_.at(i);
    if (IsAlphanumericChar(character))
      continue;
    if (frequency.find(character) == frequency.end())
      continue;
    frequency.at(character) += 1;
  }

  char_t delimiter = L'\0';

  for (const auto& pair : frequency) {
    if (pair.second == 0)
      continue;

    // Initialize delimiter at first iteration
    if (delimiter == L'\0') {
      delimiter = pair.first;
      continue;
    }

    int character_distance =
        static_cast<int>(kDelimiterTable.find(pair.first)) -
        static_cast<int>(kDelimiterTable.find(delimiter));
    // If the distance is negative, then the new delimiter has higher priority
    if (character_distance < 0 && pair.first != L',') {
      delimiter = pair.first;
      continue;
    }

    // Even if the new delimiter has lower priority, it may be much more common
    float frequency_ratio = static_cast<float>(pair.second) /
                            static_cast<float>(frequency[delimiter]);
    // The constant value was chosen by trial and error. There should be room
    // for improvement.
    // TODO: This doesn't help at all. Increasing the value has no effect,
    // while decreasing it causes more errors.
    if (frequency_ratio / abs(character_distance) > 0.8f)
      delimiter = pair.first;
  }

  return delimiter;
}

bool Tokenizer::ValidateDelimiter(const char_t delimiter, bool enclosed,
                                  const TokenRange& range) const {
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