/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <regex>

#include <anitomy/keyword.hpp>
#include <anitomy/parser.hpp>
#include <anitomy/string.hpp>

namespace anitomy {

const string_t kDashes = L"-\u2010\u2011\u2012\u2013\u2014\u2015";
const string_t kDashesWithSpace = L" -\u2010\u2011\u2012\u2013\u2014\u2015";

size_t Parser::FindNumberInString(const string_t& str) {
  auto it = std::find_if(str.begin(), str.end(), IsNumericChar);
  return it == str.end() ? str.npos : (it - str.begin());
}

string_t Parser::GetNumberFromOrdinal(const string_t& word) {
  static const std::map<string_t, string_t> ordinals{
      {L"1st", L"1"}, {L"First", L"1"},
      {L"2nd", L"2"}, {L"Second", L"2"},
      {L"3rd", L"3"}, {L"Third", L"3"},
      {L"4th", L"4"}, {L"Fourth", L"4"},
      {L"5th", L"5"}, {L"Fifth", L"5"},
      {L"6th", L"6"}, {L"Sixth", L"6"},
      {L"7th", L"7"}, {L"Seventh", L"7"},
      {L"8th", L"8"}, {L"Eighth", L"8"},
      {L"9th", L"9"}, {L"Ninth", L"9"},
  };

  auto it = ordinals.find(word);
  return it != ordinals.end() ? it->second : string_t();
}

bool Parser::IsCrc32(const string_t& str) {
  return str.size() == 8 && IsHexadecimalString(str);
}

bool Parser::IsDashCharacter(const string_t& str) {
  if (str.size() != 1)
    return false;

  auto result = std::find(kDashes.begin(), kDashes.end(), str.front());
  return result != kDashes.end();
}

bool Parser::IsResolution(const string_t& str) {
  // Using a regex such as "\\d{3,4}(p|(x\\d{3,4}))$" would be more elegant,
  // but it's much slower (e.g. 2.4ms -> 24.9ms).

  const size_t min_width_size = 3;
  const size_t min_height_size = 3;

  // *###x###*
  if (str.size() >= min_width_size + 1 + min_height_size) {
    size_t pos = str.find_first_of(L"xX\u00D7");  // multiplication sign
    if (pos != str.npos &&
        pos >= min_width_size &&
        pos <= str.size() - (min_height_size + 1)) {
      for (size_t i = 0; i < str.size(); i++)
        if (i != pos && !IsNumericChar(str.at(i)))
          return false;
      return true;
    }

  // *###p
  } else if (str.size() >= min_height_size + 1) {
    if (str.back() == L'p' || str.back() == L'P') {
      for (size_t i = 0; i < str.size() - 1; i++)
        if (!IsNumericChar(str.at(i)))
          return false;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::CheckAnimeSeasonKeyword(const token_iterator_t token) {
  auto set_anime_season = [&](token_iterator_t first, token_iterator_t second,
                              const string_t& content) {
    elements_.insert(kElementAnimeSeason, content);
    first->type = TokenType::Identifier;
    second->type = TokenType::Identifier;
  };

  auto previous_token = FindPreviousToken(tokens_, token, kFlagNotDelimiter);
  if (previous_token != tokens_.end()) {
    auto number = GetNumberFromOrdinal(previous_token->content);
    if (!number.empty()) {
      set_anime_season(previous_token, token, number);
      return true;
    }
  }

  auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);
  if (next_token != tokens_.end() &&
      IsNumericString(next_token->content)) {
    set_anime_season(token, next_token, next_token->content);
    return true;
  }

  return false;
}

bool Parser::CheckExtentKeyword(ElementCategory category,
                                const token_iterator_t token) {
  auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);

  if (CheckTokenType(next_token, TokenType::Unknown)) {
    if (FindNumberInString(next_token->content) == 0) {
      switch (category) {
        case kElementEpisodeNumber:
          if (!MatchEpisodePatterns(next_token->content, *next_token))
            SetEpisodeNumber(next_token->content, *next_token, false);
          break;
        case kElementVolumeNumber:
          if (!MatchVolumePatterns(next_token->content, *next_token))
            SetVolumeNumber(next_token->content, *next_token, false);
          break;
        default:
          return false;
      }
      token->type = TokenType::Identifier;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::IsElementCategorySearchable(ElementCategory category) {
  switch (category) {
    case kElementAnimeSeasonPrefix:
    case kElementAnimeType:
    case kElementAudioTerm:
    case kElementDeviceCompatibility:
    case kElementEpisodePrefix:
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
    case kElementVolumePrefix:
      return true;
    default:
      break;
  }

  return false;
}

bool Parser::IsElementCategorySingular(ElementCategory category) {
  switch (category) {
    case kElementAnimeSeason:
    case kElementAnimeType:
    case kElementAudioTerm:
    case kElementDeviceCompatibility:
    case kElementEpisodeNumber:
    case kElementLanguage:
    case kElementOther:
    case kElementReleaseInformation:
    case kElementSource:
    case kElementVideoTerm:
      return false;
    default:
      break;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void Parser::BuildElement(ElementCategory category, bool keep_delimiters,
                          const token_iterator_t token_begin,
                          const token_iterator_t token_end) const {
  string_t element;

  for (auto token = token_begin; token != token_end; ++token) {
    switch (token->type) {
      case TokenType::Unknown:
        element += token->content;
        token->type = TokenType::Identifier;
        break;
      case TokenType::Bracket:
        element += token->content;
        break;
      case TokenType::Delimiter: {
        auto delimiter = token->content.front();
        if (keep_delimiters) {
          element.push_back(delimiter);
        } else if (token != token_begin && token != token_end) {
          switch (delimiter) {
            case L',':
            case L'&':
              element.push_back(delimiter);
              break;
            default:
              element.push_back(L' ');
              break;
          }
        }
        break;
      }
      default:
        break;
    }
  }

  if (!keep_delimiters)
    TrimString(element, kDashesWithSpace.c_str());

  if (!element.empty())
    elements_.insert(category, element);
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::CheckTokenType(const token_iterator_t token,
                            TokenType type) const {
  return token != tokens_.end() && token->type == type;
}

bool Parser::IsTokenIsolated(const token_iterator_t token) const {
  auto previous_token = FindPreviousToken(tokens_, token, kFlagNotDelimiter);
  if (!CheckTokenType(previous_token, TokenType::Bracket))
    return false;

  auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);
  if (!CheckTokenType(next_token, TokenType::Bracket))
    return false;

  return true;
}

}  // namespace anitomy
