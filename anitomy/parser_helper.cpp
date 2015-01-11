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
#include <regex>

#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

ParseOptions::ParseOptions()
    : parse_episode_number(true),
      parse_episode_title(true),
      parse_release_group(true) {
}

////////////////////////////////////////////////////////////////////////////////

size_t Parser::FindNumberInString(const string_t& str) {
  auto it = std::find_if(str.begin(), str.end(), IsNumericChar);

  if (it == str.end()) {
    return str.npos;
  } else {
    return it - str.begin();
  }
}

bool Parser::IsCrc32(const string_t& str) {
  return str.size() == 8 && IsHexadecimalString(str);
}

bool Parser::IsDashCharacter(const string_t& str) {
  if (str.size() != 1)
    return false;

  const string_t dashes = L"-\u2010\u2011\u2012\u2013\u2014\u2015";

  auto result = std::find(dashes.begin(), dashes.end(), str.front());
  return result != dashes.end();
}

bool Parser::IsOrdinalNumber(const string_t& word) {
  using namespace std::regex_constants;

  const std::basic_regex<char_t> pattern(
      L"1st|2nd|3rd|[4-9]th|first|second|third|fourth|fifth",
      icase | nosubs | optimize);

  return std::regex_search(word, pattern, match_any | match_continuous);
}

bool Parser::IsResolution(const string_t& str) {
  // Using a regex such as "\\d{3,4}(p|(x\\d{3,4}))$" would be more elegant,
  // but it's much slower (e.g. 2.4ms -> 24.9ms).

  // *###x###*
  if (str.size() >= 3 + 1 + 3) {
    size_t pos = str.find_first_of(L"x\u00D7");  // multiplication sign
    if (pos != str.npos) {
      for (size_t i = 0; i < str.size(); i++)
        if (i != pos && !IsNumericChar(str.at(i)))
          return false;
      return true;
    }

  // *###p
  } else if (str.size() >= 3 + 1) {
    if (str.back() == L'p' || str.back() == L'P') {
      for (size_t i = 0; i < str.size() - 1; i++)
        if (!IsNumericChar(str.at(i)))
          return false;
      return true;
    }
  }

  return false;
}

bool Parser::IsElementCategorySearchable(ElementCategory category) {
  switch (category) {
    case kElementAnimeType:
    case kElementAudioTerm:
    case kElementDeviceCompatibility:
    case kElementFileChecksum:
    case kElementLanguage:
    case kElementOther:
    case kElementReleaseGroup:
    case kElementReleaseInformation:
    case kElementReleaseVersion:
    case kElementSource:
    case kElementSubtitles:
    case kElementVideoResolution:
    case kElementVideoTerm:
      return true;
  }

  return false;
}

bool Parser::IsElementCategorySingular(ElementCategory category) {
  switch (category) {
    case kElementAnimeSeason:
    case kElementAudioTerm:
    case kElementDeviceCompatibility:
    case kElementEpisodeNumber:
    case kElementLanguage:
    case kElementOther:
    case kElementReleaseInformation:
    case kElementSource:
    case kElementVideoTerm:
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

token_iterator_t Parser::GetPreviousValidToken(token_iterator_t it) const {
  if (it == tokens_.begin())
    return tokens_.end();

  // Find the first previous token that's not a delimiter
  do {
    --it;
  } while (it != tokens_.begin() && it->category == kDelimiter);

  return it;
}

token_iterator_t Parser::GetNextValidToken(token_iterator_t it) const {
  // Find the first next token that's not a delimiter
  do {
    ++it;
  } while (it != tokens_.end() && it->category == kDelimiter);

  return it;
}

////////////////////////////////////////////////////////////////////////////////

void Parser::BuildElement(ElementCategory category, bool keep_delimiters,
                          const token_iterator_t& token_begin,
                          const token_iterator_t& token_end) const {
  string_t element;

  for (auto token = token_begin; token != token_end; ++token) {
    switch (token->category) {
      case kUnknown:
        element += token->content;
        token->category = kIdentifier;
        break;
      case kBracket:
        element += token->content;
        break;
      case kDelimiter:
        if (keep_delimiters) {
          element.push_back(token->content.front());
        } else if (token != token_begin && token != token_end) {
          element.push_back(L' ');
        }
        break;
    }
  }

  if (!keep_delimiters)
    TrimString(element, L" -\u2010\u2011\u2012\u2013\u2014\u2015");

  elements_.insert(category, element);
}

}  // namespace anitomy