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

#include <algorithm>
#include <regex>

#include "element.h"
#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

bool Parser::IsValidEpisodeNumber(const string_t& number) {
  return StringToInt(number) <= kEpisodeNumberMax;
}

bool Parser::SetEpisodeNumber(const string_t& number, Token& token,
                              bool validate) {
  if (validate)
    if (!IsValidEpisodeNumber(number))
      return false;

  elements_.insert(kElementEpisodeNumber, number);
  token.category = kIdentifier;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::NumberComesAfterEpisodePrefix(Token& token) {
  size_t number_begin = FindNumberInString(token.content);
  auto prefix = keyword_manager.Normalize(token.content.substr(0, number_begin));

  if (keyword_manager.Find(kElementEpisodePrefix, prefix)) {
    auto number = token.content.substr(
        number_begin, token.content.length() - number_begin);
    if (!MatchEpisodePatterns(number, token))
      SetEpisodeNumber(number, token, false);
    return true;
  }

  return false;
}

bool Parser::NumberComesBeforeTotalNumber(const token_iterator_t token) {
  auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);

  if (next_token != tokens_.end()) {
    if (IsStringEqualTo(next_token->content, L"of")) {
      auto other_token = FindNextToken(tokens_, next_token, kFlagNotDelimiter);

      if (other_token != tokens_.end()) {
        if (IsNumericString(other_token->content)) {
          SetEpisodeNumber(token->content, *token, false);
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
  for (const auto& token_index : tokens) {
    auto token = tokens_.begin() + token_index;
    bool numeric_front = IsNumericChar(token->content.front());

    if (!numeric_front) {
      // e.g. "EP.01"
      if (NumberComesAfterEpisodePrefix(*token))
        return true;
    } else {
      // e.g. "8 of 12"
      if (NumberComesBeforeTotalNumber(token))
        return true;
    }
    // Look for other patterns
    if (MatchEpisodePatterns(token->content, *token))
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

typedef std::basic_regex<char_t> regex_t;
typedef std::match_results<string_t::const_iterator> regex_match_results_t;

bool Parser::MatchSingleEpisodePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,3})[vV](\\d)");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetEpisodeNumber(match_results[1].str(), token, false);
    elements_.insert(kElementReleaseVersion, match_results[2].str());
    return true;
  }

  return false;
}

bool Parser::MatchMultiEpisodePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,3})[-~&+](\\d{1,3})(?:[vV](\\d))?");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    auto lower_bound = match_results[1].str();
    auto upper_bound = match_results[2].str();
    // Avoid matching expressions such as "009-1" or "5-2"
    if (StringToInt(lower_bound) < StringToInt(upper_bound)) {
      if (SetEpisodeNumber(lower_bound, token, true)) {
        SetEpisodeNumber(upper_bound, token, false);
        if (match_results[3].matched)
          elements_.insert(kElementReleaseVersion, match_results[3].str());
        return true;
      }
    }
  }

  return false;
}

bool Parser::MatchSeasonAndEpisodePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"S?"
                               L"(\\d{1,2})(?:-S?(\\d{1,2}))?"
                               L"(?:x|[ ._-x]?E)"
                               L"(\\d{1,3})(?:-E?(\\d{1,3}))?",
                               std::regex_constants::icase);
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    elements_.insert(kElementAnimeSeason, match_results[1]);
    if (match_results[2].matched)
      elements_.insert(kElementAnimeSeason, match_results[2]);
    SetEpisodeNumber(match_results[3], token, false);
    if (match_results[4].matched)
      SetEpisodeNumber(match_results[4], token, false);
    return true;
  }

  return false;
}

bool Parser::MatchTypeAndEpisodePattern(const string_t& word, Token& token) {
  size_t number_begin = FindNumberInString(word);
  auto prefix = word.substr(0, number_begin);

  ElementCategory category = kElementAnimeType;
  KeywordOptions options;

  if (keyword_manager.Find(keyword_manager.Normalize(prefix),
                           category, options)) {
    elements_.insert(kElementAnimeType, prefix);
    auto number = word.substr(number_begin);
    if (MatchEpisodePatterns(number, token) ||
        SetEpisodeNumber(number, token, true)) {
      auto it = std::find(tokens_.begin(), tokens_.end(), token);
      if (it != tokens_.end()) {
        // Split token (we do this last in order to avoid invalidating our
        // token reference earlier)
        token.content = number;
        tokens_.insert(it, Token(options.identifiable ? kIdentifier : kUnknown,
                                 prefix, token.enclosed));
      }
      return true;
    }
  }

  return false;
}

bool Parser::MatchFractionalEpisodePattern(const string_t& word, Token& token) {
  // We don't allow any fractional part other than ".5", because there are cases
  // where such a number is a part of the anime title (e.g. "Evangelion: 1.11",
  // "Tokyo Magnitude 8.0") or a keyword (e.g. "5.1").
  static const regex_t pattern(L"\\d+\\.5");

  if (std::regex_match(word, pattern))
    if (SetEpisodeNumber(word, token, true))
      return true;

  return false;
}

bool Parser::MatchPartialEpisodePattern(const string_t& word, Token& token) {
  auto it = std::find_if_not(word.begin(), word.end(), IsNumericChar);
  auto suffix_length = std::distance(it, word.end());

  auto is_valid_suffix = [](const char_t c) {
    return (c >= L'A' && c <= L'C') ||
           (c >= L'a' && c <= L'c');
  };

  if (suffix_length == 1 && is_valid_suffix(*it))
    if (SetEpisodeNumber(word, token, true))
      return true;

  return false;
}

bool Parser::MatchNumberSignPattern(const string_t& word, Token& token) {
  if (word.front() != L'#')
    return false;

  static const regex_t pattern(L"#(\\d{1,3})(?:[-~&+](\\d{1,3}))?(?:[vV](\\d))?");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    if (SetEpisodeNumber(match_results[1].str(), token, true)) {
      if (match_results[2].matched)
        SetEpisodeNumber(match_results[2].str(), token, false);
      if (match_results[3].matched)
        elements_.insert(kElementReleaseVersion, match_results[3].str());
      return true;
    }
  }

  return false;
}

bool Parser::MatchJapaneseCounterPattern(const string_t& word, Token& token) {
  if (word.back() != L'\u8A71')
    return false;

  static const regex_t pattern(L"(\\d{1,3})\u8A71");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetEpisodeNumber(match_results[1].str(), token, false);
    return true;
  }

  return false;
}

bool Parser::MatchEpisodePatterns(string_t word, Token& token) {
  // All patterns contain at least one non-numeric character
  if (IsNumericString(word))
    return false;

  TrimString(word, L" -");

  const bool numeric_front = IsNumericChar(word.front());
  const bool numeric_back = IsNumericChar(word.back());

  // e.g. "01v2"
  if (numeric_front && numeric_back)
    if (MatchSingleEpisodePattern(word, token))
      return true;
  // e.g. "01-02", "03-05v2"
  if (numeric_front && numeric_back)
    if (MatchMultiEpisodePattern(word, token))
      return true;
  // e.g. "2x01", "S01E03", "S01-02xE001-150"
  if (numeric_back)
    if (MatchSeasonAndEpisodePattern(word, token))
      return true;
  // e.g. "ED1", "OP4a", "OVA2"
  if (!numeric_front)
    if (MatchTypeAndEpisodePattern(word, token))
      return true;
  // e.g. "07.5"
  if (numeric_front && numeric_back)
    if (MatchFractionalEpisodePattern(word, token))
      return true;
  // e.g. "4a", "111C"
  if (numeric_front && !numeric_back)
    if (MatchPartialEpisodePattern(word, token))
      return true;
  // e.g. "#01", "#02-03v2"
  if (numeric_back)
    if (MatchNumberSignPattern(word, token))
      return true;
  // U+8A71 is used as counter for stories, episodes of TV series, etc.
  if (numeric_front)
    if (MatchJapaneseCounterPattern(word, token))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::SearchForEquivalentNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;

    if (IsTokenIsolated(token))
      continue;

    // Find the first enclosed, non-delimiter token
    auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);
    if (next_token != tokens_.end() &&
        next_token->category == kBracket) {
      next_token = FindNextToken(tokens_, next_token,
                                 kFlagEnclosed | kFlagNotDelimiter);
    } else {
      continue;
    }

    // See if it's an isolated number
    if (next_token != tokens_.end() &&
        next_token->category == kUnknown &&
        IsTokenIsolated(next_token) &&
        IsNumericString(next_token->content)) {
      if (IsValidEpisodeNumber(token->content) &&
          IsValidEpisodeNumber(next_token->content)) {
        auto lower_token =
            StringToInt(token->content) < StringToInt(next_token->content) ?
            token : next_token;
        SetEpisodeNumber(lower_token->content, *token, false);
        next_token->category = kIdentifier;
        return true;
      }
    }
  }

  return false;
}

bool Parser::SearchForIsolatedNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;

    if (!token->enclosed || !IsTokenIsolated(token))
      continue;

    if (SetEpisodeNumber(token->content, *token, true))
      return true;
  }

  return false;
}

bool Parser::SearchForSeparatedNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;
    auto previous_token = FindPreviousToken(tokens_, token, kFlagNotDelimiter);

    // See if the number has a preceding "-" separator
    if (previous_token != tokens_.end() &&
        previous_token->category == kUnknown &&
        IsDashCharacter(previous_token->content)) {
      if (SetEpisodeNumber(token->content, *token, true)) {
        previous_token->category = kIdentifier;
        return true;
      }
    }
  }

  return false;
}

bool Parser::SearchForLastNumber(std::vector<size_t>& tokens) {
  for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
    size_t token_index = *it;
    auto token = tokens_.begin() + token_index;

    // Assuming that episode number always comes after the title, first token
    // cannot be what we're looking for
    if (token_index == 0)
      continue;

    // An enclosed token is unlikely to be the episode number at this point
    if (token->enclosed)
      continue;

    // Ignore if it's the first non-enclosed, non-delimiter token
    if (std::all_of(tokens_.begin(), token, [](const Token& token) {
            return token.enclosed || token.category == kDelimiter; }))
      continue;

    // Ignore if the previous token is "Movie" or "Part"
    auto previous_token = FindPreviousToken(tokens_, token, kFlagNotDelimiter);
    if (previous_token != tokens_.end() &&
        previous_token->category == kUnknown) {
      if (IsStringEqualTo(previous_token->content, L"Movie") ||
          IsStringEqualTo(previous_token->content, L"Part")) {
        continue;
      }
    }

    // We'll use this number after all
    if (SetEpisodeNumber(token->content, *token, true))
      return true;
  }

  return false;
}

}  // namespace anitomy