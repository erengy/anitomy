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

#include "element.h"
#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

void Parser::SetEpisodeNumber(string_t number, Token& token) {
  TrimString(number);

  elements_.Add(kElementEpisodeNumber, number);

  token.category = kIdentifier;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::NumberComesAfterEpisodePrefix(Token& token, size_t number_begin) {
  auto prefix = StringToUpperCopy(token.content.substr(0, number_begin));

  if (keyword_manager.Find(kElementEpisodePrefix, prefix)) {
    auto number = token.content.substr(
        number_begin, token.content.length() - number_begin);
    if (!MatchEpisodePatterns(number, token))
      SetEpisodeNumber(number, token);
    return true;
  }

  return false;
}

bool Parser::NumberComesAfterEpisodeKeyword(const token_iterator_t& token) {
  auto previous_token = GetPreviousValidToken(token);

  if (previous_token != tokens_.end()) {
    if (previous_token->category == kUnknown) {
      auto keyword = StringToUpperCopy(previous_token->content);

      if (keyword_manager.Find(kElementEpisodePrefix, keyword)) {
        if (!MatchEpisodePatterns(token->content, *token))
          SetEpisodeNumber(token->content, *token);
        previous_token->category = kIdentifier;
        return true;
      }
    }
  }

  return false;
}

bool Parser::NumberComesBeforeTotalNumber(const token_iterator_t& token) {
  auto next_token = GetNextValidToken(token);

  if (next_token != tokens_.end()) {
    if (IsStringEqualTo(next_token->content, _TEXT("of"))) {
      auto other_token = GetNextValidToken(next_token);

      if (other_token != tokens_.end()) {
        if (IsNumericString(other_token->content)) {
          SetEpisodeNumber(token->content, *token);
          next_token->category = kIdentifier;
          other_token->category = kIdentifier;
          return true;
        }
      }
    }
  }

  return false;
}

bool Parser::SearchForEpisodePatterns(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;
    size_t number_begin = FindNumberInString(token->content);

    if (number_begin > 0) {
      // e.g. "EP.01"
      if (NumberComesAfterEpisodePrefix(*token, number_begin))
        return true;
    } else {
      // e.g. "Episode 01"
      if (NumberComesAfterEpisodeKeyword(token))
        return true;
      // e.g. "8 of 12"
      if (NumberComesBeforeTotalNumber(token))
        return true;
      // Look for other patterns
      if (MatchEpisodePatterns(token->content, *token))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

typedef std::basic_regex<char_t> regex_t;
typedef std::match_results<string_t::const_iterator> regex_match_results_t;

bool Parser::MatchSingleEpisodePattern(const string_t& word, Token& token) {
  static regex_t pattern(_TEXT("(\\d+)v(\\d)"));
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetEpisodeNumber(match_results[1].str(), token);
    elements_.Add(kElementReleaseVersion, match_results[2].str());
    return true;
  }

  return false;
}

bool Parser::MatchMultiEpisodePattern(const string_t& word, Token& token) {
  static regex_t pattern(_TEXT("(\\d+)[-&+](\\d+)(v(\\d))?"));
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    auto lower_bound = match_results[1].str();
    auto upper_bound = match_results[2].str();
    // We're checking bounds to avoid matching expressions such as "009-1"
    if (StringToInt(lower_bound) < StringToInt(upper_bound)) {
      SetEpisodeNumber(lower_bound + _TEXT("-") + upper_bound, token);
      if (match_results[4].matched)
        elements_.Add(kElementReleaseVersion, match_results[4].str());
      return true;
    }
  }

  return false;
}

bool Parser::MatchSeasonAndEpisodePattern(const string_t& word, Token& token) {
  static regex_t pattern(_TEXT("S?(\\d{1,2})xE?(\\d{1,2})"));
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    elements_.Add(kElementAnimeSeason, match_results[1]);
    SetEpisodeNumber(match_results[2], token);
    return true;
  }

  return false;
}

bool Parser::MatchJapaneseCounterPattern(const string_t& word, Token& token) {
  static regex_t pattern(_TEXT("(\\d+)\u8A71"));
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetEpisodeNumber(match_results[1].str(), token);
    return true;
  }

  return false;
}

bool Parser::MatchEpisodePatterns(const string_t& word, Token& token) {
  // e.g. "01v2"
  if (MatchSingleEpisodePattern(word, token))
    return true;
  // e.g. "01-02", "03-05v2"
  if (MatchMultiEpisodePattern(word, token))
    return true;
  // e.g. "02x01", "S01xE03"
  if (MatchSeasonAndEpisodePattern(word, token))
    return true;
  // U+8A71 is used as counter for stories, episodes of TV series, etc.
  if (MatchJapaneseCounterPattern(word, token))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::SearchForIsolatedNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;
    auto previous_token = GetPreviousValidToken(token);

    if (previous_token != tokens_.end() &&
        previous_token->category == kBracket) {
      auto next_token = GetNextValidToken(token);

      if (next_token != tokens_.end() &&
          next_token->category == kBracket) {
        auto number = StringToInt(token->content);

        // While there are about a dozen anime series with more than 1000
        // episodes (e.g. Doraemon), it's safe to assume that any number within
        // the interval is not the episode number.
        if (number > 1900 && number < 2050) {
          elements_.Add(kElementAnimeYear, token->content);
          // We don't set token category to identifier here, because there might
          // be a good reason to keep the year as a part of the title, as in
          // "Fullmetal Alchemist (2009)".
        } else if (number <= 1900) {
          SetEpisodeNumber(token->content, *token);
          return true;
        }
      }
    }
  }

  return false;
}

bool Parser::SearchForSeparatedNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;
    auto previous_token = GetPreviousValidToken(token);

    // See if the number has a preceding "-" separator
    if (previous_token != tokens_.end() &&
        previous_token->category == kUnknown &&
        previous_token->content == _TEXT("-")) {
      SetEpisodeNumber(token->content, *token);
      previous_token->category = kIdentifier;
      return true;
    }
  }

  return false;
}

bool Parser::SearchForLastNumber(std::vector<size_t>& tokens) {
  for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
    size_t token_index = *it;
    auto& token = tokens_.begin() + token_index;

    // Assuming that episode number always comes after the title, first token
    // cannot be what we're looking for
    if (token_index == 0)
      continue;

    // An enclosed token is unlikely to be the episode number at this point
    if (token->enclosed)
      continue;

    // Ignore if it's the first non-enclosed token
    if (std::all_of(tokens_.begin(), tokens_.begin() + token_index,
            [](const Token& token) { return token.enclosed; }))
      continue;

    // Ignore if there's an identified token placed before this one
    if (std::any_of(tokens_.begin(), tokens_.begin() + token_index,
            [](const Token& token) { return token.category == kIdentifier; }))
      continue;

    // Check if the previous token is "Season" or "Movie"
    auto previous_token = GetPreviousValidToken(token);
    if (previous_token != tokens_.end() &&
        previous_token->category == kUnknown) {
      if (IsStringEqualTo(previous_token->content, _TEXT("Season"))) {
        // We can't bail out yet; it can still be in "2nd Season 01" format
        previous_token = GetPreviousValidToken(previous_token);
        if (previous_token != tokens_.end()) {
          if (IsOrdinalNumber(previous_token->content)) {
            elements_.Add(kElementAnimeSeason, previous_token->content);
          } else {
            elements_.Add(kElementAnimeSeason, token->content);
            continue;
          }
        }
      } else if (IsStringEqualTo(previous_token->content, _TEXT("Movie"))) {
        continue;
      }
    }

    // We'll use this number after all
    SetEpisodeNumber(token->content, *token);
    return true;
  }

  return false;
}

}  // namespace anitomy