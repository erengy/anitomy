/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <regex>

#include <anitomy/element.hpp>
#include <anitomy/keyword.hpp>
#include <anitomy/parser.hpp>
#include <anitomy/string.hpp>
#include <anitomy/util.hpp>

namespace anitomy {

bool Parser::IsValidEpisodeNumber(const string_t& number) {
  return StringToInt(number) <= kEpisodeNumberMax;
}

bool Parser::SetEpisodeNumber(const string_t& number, Token& token,
                              bool validate) {
  if (validate && !IsValidEpisodeNumber(number))
    return false;

  token.type = TokenType::Identifier;

  auto type = ElementType::EpisodeNumber;

  // Handle equivalent numbers
  if (found_episode_keywords_) {
    for (auto& element : elements_) {
      if (element.type != ElementType::EpisodeNumber)
        continue;
      // The larger number gets to be the alternative one
      const int comparison = StringToInt(number) - StringToInt(element.value);
      if (comparison > 0) {
        type = ElementType::EpisodeNumberAlt;
      } else if (comparison < 0) {
        element.type = ElementType::EpisodeNumberAlt;
      } else {
        return false;  // No need to add the same number twice
      }
      break;
    }
  }

  elements_.insert(type, number);
  return true;
}

bool Parser::SetAlternativeEpisodeNumber(const string_t& number, Token& token) {
  elements_.insert(ElementType::EpisodeNumberAlt, number);
  token.type = TokenType::Identifier;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::IsValidVolumeNumber(const string_t& number) {
  return StringToInt(number) <= kVolumeNumberMax;
}

bool Parser::SetVolumeNumber(const string_t& number, Token& token,
                             bool validate) {
  if (validate)
    if (!IsValidVolumeNumber(number))
      return false;

  elements_.insert(ElementType::VolumeNumber, number);
  token.type = TokenType::Identifier;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::NumberComesAfterPrefix(ElementType type, Token& token) {
  size_t number_begin = FindNumberInString(token.value);
  auto prefix = keyword_manager.Normalize(token.value.substr(0, number_begin));

  if (keyword_manager.Find(type, prefix)) {
    auto number = token.value.substr(
        number_begin, token.value.length() - number_begin);
    switch (type) {
      case ElementType::EpisodePrefix:
        if (!MatchEpisodePatterns(number, token))
          SetEpisodeNumber(number, token, false);
        return true;
      case ElementType::VolumePrefix:
        if (!MatchVolumePatterns(number, token))
          SetVolumeNumber(number, token, false);
        return true;
      default:
        break;
    }
  }

  return false;
}

bool Parser::NumberComesBeforeAnotherNumber(const Tokens::iterator token) {
  auto separator_token = FindNextToken(tokens_, token, kFlagNotDelimiter);

  if (separator_token != tokens_.end()) {
    static const std::vector<std::pair<string_t, bool>> separators{
      {L"&", true}, {L"of", false},
    };
    for (const auto& separator : separators) {
      if (IsStringEqualTo(separator_token->value, separator.first)) {
        auto other_token = FindNextToken(tokens_, separator_token, kFlagNotDelimiter);
        if (other_token != tokens_.end() &&
            IsNumericString(other_token->value)) {
          SetEpisodeNumber(token->value, *token, false);
          if (separator.second)
            SetEpisodeNumber(other_token->value, *other_token, false);
          separator_token->type = TokenType::Identifier;
          other_token->type = TokenType::Identifier;
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
    bool numeric_front = IsNumericChar(token->value.front());

    if (!numeric_front) {
      // e.g. "EP.1", "Vol.1"
      if (NumberComesAfterPrefix(ElementType::EpisodePrefix, *token))
        return true;
      if (NumberComesAfterPrefix(ElementType::VolumePrefix, *token))
        continue;
    } else {
      // e.g. "8 & 10", "01 of 24"
      if (NumberComesBeforeAnotherNumber(token))
        return true;
    }
    // Look for other patterns
    if (MatchEpisodePatterns(token->value, *token))
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

using regex_t = std::basic_regex<char_t>;
using regex_match_results_t = std::match_results<string_t::const_iterator>;

bool Parser::MatchSingleEpisodePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,3})[vV](\\d)");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetEpisodeNumber(match_results[1].str(), token, false);
    elements_.insert(ElementType::ReleaseVersion, match_results[2].str());
    return true;
  }

  return false;
}

bool Parser::MatchMultiEpisodePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,3})(?:[vV](\\d))?[-~&+]"
                               L"(\\d{1,3})(?:[vV](\\d))?");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    auto lower_bound = match_results[1].str();
    auto upper_bound = match_results[3].str();
    // Avoid matching expressions such as "009-1" or "5-2"
    if (StringToInt(lower_bound) < StringToInt(upper_bound)) {
      if (SetEpisodeNumber(lower_bound, token, true)) {
        SetEpisodeNumber(upper_bound, token, false);
        if (match_results[2].matched)
          elements_.insert(ElementType::ReleaseVersion, match_results[2].str());
        if (match_results[4].matched)
          elements_.insert(ElementType::ReleaseVersion, match_results[4].str());
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
    elements_.insert(ElementType::AnimeSeason, match_results[1]);
    if (match_results[2].matched)
      elements_.insert(ElementType::AnimeSeason, match_results[2]);
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

  ElementType type = ElementType::AnimeType;
  KeywordOptions options;

  if (keyword_manager.Find(keyword_manager.Normalize(prefix),
                           type, options)) {
    elements_.insert(ElementType::AnimeType, prefix);
    auto number = word.substr(number_begin);
    if (MatchEpisodePatterns(number, token) ||
        SetEpisodeNumber(number, token, true)) {
      auto it = std::find(tokens_.begin(), tokens_.end(), token);
      if (it != tokens_.end()) {
        // Split token (we do this last in order to avoid invalidating our
        // token reference earlier)
        token.value = number;
        tokens_.emplace(it,
            options.identifiable ? TokenType::Identifier : TokenType::Unknown,
            prefix, token.enclosed);
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
        elements_.insert(ElementType::ReleaseVersion, match_results[3].str());
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

bool Parser::MatchSingleVolumePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,2})[vV](\\d)");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    SetVolumeNumber(match_results[1].str(), token, false);
    elements_.insert(ElementType::ReleaseVersion, match_results[2].str());
    return true;
  }

  return false;
}

bool Parser::MatchMultiVolumePattern(const string_t& word, Token& token) {
  static const regex_t pattern(L"(\\d{1,2})[-~&+](\\d{1,2})(?:[vV](\\d))?");
  regex_match_results_t match_results;

  if (std::regex_match(word, match_results, pattern)) {
    auto lower_bound = match_results[1].str();
    auto upper_bound = match_results[2].str();
    if (StringToInt(lower_bound) < StringToInt(upper_bound)) {
      if (SetVolumeNumber(lower_bound, token, true)) {
        SetVolumeNumber(upper_bound, token, false);
        if (match_results[3].matched)
          elements_.insert(ElementType::ReleaseVersion, match_results[3].str());
        return true;
      }
    }
  }

  return false;
}

bool Parser::MatchVolumePatterns(string_t word, Token& token) {
  // All patterns contain at least one non-numeric character
  if (IsNumericString(word))
    return false;

  TrimString(word, L" -");

  const bool numeric_front = IsNumericChar(word.front());
  const bool numeric_back = IsNumericChar(word.back());

  // e.g. "01v2"
  if (numeric_front && numeric_back)
    if (MatchSingleVolumePattern(word, token))
      return true;
  // e.g. "01-02", "03-05v2"
  if (numeric_front && numeric_back)
    if (MatchMultiVolumePattern(word, token))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::SearchForEquivalentNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;

    if (IsTokenIsolated(token) || !IsValidEpisodeNumber(token->value))
      continue;

    // Find the first enclosed, non-delimiter token
    auto next_token = FindNextToken(tokens_, token, kFlagNotDelimiter);
    if (!CheckTokenType(next_token, TokenType::Bracket))
      continue;
    next_token = FindNextToken(tokens_, next_token,
                               kFlagEnclosed | kFlagNotDelimiter);
    if (!CheckTokenType(next_token, TokenType::Unknown))
      continue;

    // Check if it's an isolated number
    if (!IsTokenIsolated(next_token) ||
        !IsNumericString(next_token->value) ||
        !IsValidEpisodeNumber(next_token->value))
      continue;

    auto minmax = std::minmax(token, next_token,
        [](const Tokens::iterator& a, const Tokens::iterator& b) {
          return StringToInt(a->value) < StringToInt(b->value);
        });
    SetEpisodeNumber(minmax.first->value, *minmax.first, false);
    SetAlternativeEpisodeNumber(minmax.second->value, *minmax.second);

    return true;
  }

  return false;
}

bool Parser::SearchForIsolatedNumbers(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_.begin() + *token_index;

    if (!token->enclosed || !IsTokenIsolated(token))
      continue;

    if (SetEpisodeNumber(token->value, *token, true))
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
    if (CheckTokenType(previous_token, TokenType::Unknown) &&
        IsDashCharacter(previous_token->value)) {
      if (SetEpisodeNumber(token->value, *token, true)) {
        previous_token->type = TokenType::Identifier;
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
            return token.enclosed || token.type == TokenType::Delimiter; }))
      continue;

    // Ignore if the previous token is "Movie" or "Part"
    auto previous_token = FindPreviousToken(tokens_, token, kFlagNotDelimiter);
    if (CheckTokenType(previous_token, TokenType::Unknown)) {
      if (IsStringEqualTo(previous_token->value, L"Movie") ||
          IsStringEqualTo(previous_token->value, L"Part")) {
        continue;
      }
    }

    // We'll use this number after all
    if (SetEpisodeNumber(token->value, *token, true))
      return true;
  }

  return false;
}

}  // namespace anitomy
