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
      parse_extra_keywords(true),
      parse_release_group(true) {
}

token_iterator_t Parser::GetPreviousValidToken(token_iterator_t it) const {
  if (it == tokens_->begin())
    return tokens_->end();

  // Find the first previous token that's not a delimiter
  do {
    --it;
  } while (it != tokens_->begin() && it->category == kDelimiter);

  return it;
}

token_iterator_t Parser::GetNextValidToken(token_iterator_t it) const {
  // Find the first next token that's not a delimiter
  do {
    ++it;
  } while (it != tokens_->end() && it->category == kDelimiter);

  return it;
}

// TODO: use vector instead
void AppendKeyword(string_t& str, const string_t& keyword) {
  if (keyword.empty())
    return;

  if (!str.empty())
    str.append(_TEXT(" "));

  str.append(keyword);
}

bool IsCountingWord(const string_t& str) {
  using namespace std::tr1::regex_constants;

  const std::basic_regex<char_t> pattern(
      _TEXT("1st|2nd|3rd|[4-9]th|first|second|third|fourth|fifth"),
      icase | nosubs | optimize);

  return std::regex_search(str, pattern, match_any | match_continuous);
}

bool IsCrc32(const string_t& str) {
  return str.size() == 8 && IsHexadecimalString(str);
}

bool IsResolution(const string_t& str) {
  // Using a regex such as "\\d{3,4}(p|(x\\d{3,4}))$" would be more elegant,
  // but it's much slower (e.g. 2.4ms -> 24.9ms).

  // *###x###*
  if (str.size() >= 3 + 1 + 3) {
    size_t pos = str.find('x');
    if (pos != str.npos) {
      for (size_t i = 0; i < str.size(); i++)
        if (i != pos && !IsNumericChar(str.at(i)))
          return false;
      return true;
    }

  // *###p
  } else if (str.size() >= 3 + 1) {
    if (str.back() == 'p') {
      for (size_t i = 0; i < str.size() - 1; i++)
        if (!IsNumericChar(str.at(i)))
          return false;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

Parser::Parser(Elements& data, token_container_t& tokens)
    : data_(&data),
      tokens_(&tokens) {
}

bool Parser::Parse() {
  for (auto token = tokens_->begin(); token != tokens_->end(); ++token) {
    CompareTokenWithKeywords(*token);
  }

  if (parse_options.parse_episode_number) {
    SearchForEpisodeNumber();
  }

  SearchForAnimeTitle();

  if (parse_options.parse_release_group &&
      data_->release_group.empty()) {
    SearchForReleaseGroup();
  }

  if (parse_options.parse_episode_title) {
    SearchForEpisodeTitle();
  }

  return !data_->anime_title.empty();
}

void Parser::CompareTokenWithKeywords(Token& token) {
  if (token.category != kUnknown)
    return;

  auto word = token.content;
  TrimString(word);

  // Don't bother if the word is a number that cannot be CRC
  if (word.size() != 8 && IsNumericString(word))
    return;

  // Performs better than making a case-insensitive Find
  auto keyword = StringToUpperCopy(token.content);
  TrimString(keyword);

  // Checksum
  if (data_->checksum.empty() && IsCrc32(word)) {
    data_->checksum = word;
    token.category = kIdentifier;

  // Video resolution
  } else if (data_->resolution.empty() && IsResolution(word)) {
    data_->resolution = word;
    token.category = kIdentifier;

  // Video info
  } else if (keyword_manager.Find(kKeywordVideo, keyword)) {
    AppendKeyword(data_->video, word);
    token.category = kIdentifier;

  // Audio info
  } else if (keyword_manager.Find(kKeywordAudio, keyword)) {
    AppendKeyword(data_->audio, word);
    token.category = kIdentifier;

  // Version
  } else if (data_->release_version.empty() &&
             keyword_manager.Find(kKeywordVersion, keyword)) {
    data_->release_version.push_back(word.back());  // number without "v"
    token.category = kIdentifier;

  // Group
  } else if (parse_options.parse_release_group &&
             data_->release_group.empty() &&
             keyword_manager.Find(kKeywordGroup, keyword)) {
    data_->release_group = word;
    token.category = kIdentifier;

  // Extras
  } else if (parse_options.parse_extra_keywords &&
             keyword_manager.Find(kKeywordExtra, keyword)) {
    AppendKeyword(data_->extras, word);
    token.category = kIdentifier;
  } else if (parse_options.parse_extra_keywords &&
             keyword_manager.Find(kKeywordExtraUnsafe, keyword)) {
    AppendKeyword(data_->extras, word);
    if (token.enclosed)
      token.category = kIdentifier;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool Parser::SearchForKnownEpisodeFormats(std::vector<size_t>& tokens) {
  for (auto token_index = tokens.begin();
       token_index != tokens.end(); ++token_index) {
    auto token = tokens_->begin() + *token_index;

    // Find the first numeral
    size_t number_begin = 0;
    while (number_begin < token->content.size() &&
           !IsNumericChar(token->content.at(number_begin))) {
      number_begin++;
    }

    // Begins with episode prefix (e.g. "EP.01")
    if (number_begin > 0) {
      auto prefix = StringToUpperCopy(token->content.substr(0, number_begin));
      if (keyword_manager.Find(kKeywordEpisodePrefix, prefix)) {
        auto number = token->content.substr(
            number_begin, token->content.length() - number_begin);
        SetEpisodeNumber(number, *token);
        MatchKnownEpisodePatterns(number, token);
        return true;
      }
      continue;  // All other cases begin with a number
    }

    // Previous token as episode prefix (e.g. "Episode 01")
    auto previous_token = GetPreviousValidToken(token);
    if (previous_token != tokens_->end()) {
      if (previous_token->category == kUnknown) {
        auto prefix = StringToUpperCopy(previous_token->content);
        if (keyword_manager.Find(kKeywordEpisodePrefix, prefix)) {
          SetEpisodeNumber(token->content, *token);
          MatchKnownEpisodePatterns(token->content, token);
          previous_token->category = kIdentifier;
          return true;
        }
      }
    }

    // Number and total format (e.g. "8 of 12")
    auto next_token = GetNextValidToken(token);
    if (next_token != tokens_->end()) {
      if (IsStringEqualTo(next_token->content, _TEXT("of"))) {
        auto other_token = GetNextValidToken(next_token);
        if (other_token != tokens_->end()) {
          if (IsNumericString(other_token->content)) {
            SetEpisodeNumber(token->content, *token);
            (token + 1)->category = kIdentifier;
            next_token->category = kIdentifier;
            return true;
          }
        }
      }
    }

    if (MatchKnownEpisodePatterns(token->content, token))
      return true;
  }

  return false;
}

bool Parser::MatchKnownEpisodePatterns(const string_t& str,
                                       const token_iterator_t& token) {
  static std::basic_regex<char_t> single_episode_pattern(
      _TEXT("(\\d+)v(\\d)"));
  static std::basic_regex<char_t> multi_episode_pattern(
      _TEXT("(\\d+)[-&+](\\d+)(v(\\d))?"));
  static std::basic_regex<char_t> season_and_episode_pattern(
      _TEXT("S?(\\d{1,2})xE?(\\d{1,2})"));
  static std::basic_regex<char_t> japanese_counter_pattern(
      _TEXT("(\\d+)\u8A71"));

  std::match_results<string_t::const_iterator> match_result;

  // Single episode (e.g. "01v2")
  if (std::regex_match(str, match_result, single_episode_pattern)) {
    SetEpisodeNumber(match_result[1].str(), *token);
    data_->release_version = match_result[2].str();
    return true;
  }

  // Multi episode (e.g. "01-02", "03-05v2")
  if (std::regex_match(str, match_result, multi_episode_pattern)) {
    auto lower_bound = match_result[1].str();
    auto upper_bound = match_result[2].str();
    // Avoid matching "009-1"
    if (StringToInt(lower_bound) < StringToInt(upper_bound)) {
      SetEpisodeNumber(lower_bound + _TEXT("-") + upper_bound, *token);
      if (match_result.size() > 4)
        data_->release_version = match_result[4].str();
      return true;
    }
  }

  // Season and episode (e.g. "02x01")
  if (std::regex_match(str, match_result, season_and_episode_pattern)) {
    data_->anime_season = match_result[1];
    SetEpisodeNumber(match_result[2], *token);
    return true;
  }

  // Japanese counter
  // (U+8A71 is used as counter for stories, episodes of TV series, etc.)
  if (std::regex_match(str, match_result, japanese_counter_pattern)) {
    SetEpisodeNumber(match_result[1].str(), *token);
    return true;
  }

  return false;
}

void Parser::SearchForEpisodeNumber() {
  std::vector<size_t> tokens_with_numbers;

  // List all tokens that contain a number
  for (size_t i = 0; i < tokens_->size(); i++) {
    auto& token = tokens_->at(i);
    if (token.category != kUnknown)
      continue;  // Skip previously identified tokens
    if (std::find_if(token.content.begin(), token.content.end(),
                     IsNumericChar) != token.content.end()) {
      tokens_with_numbers.push_back(i);
    }
  }
  if (tokens_with_numbers.empty())
    return;

  // If in a known episode format, it has to be the episode number
  if (SearchForKnownEpisodeFormats(tokens_with_numbers))
    return;

  // From now on, we're only interested in tokens that are entirely numeric
  for (auto it = tokens_with_numbers.begin();
       it != tokens_with_numbers.end(); ) {
    if (!IsNumericString(tokens_->at(*it).content)) {
      it = tokens_with_numbers.erase(it);  // Discard
    } else {
      ++it;
    }
  }
  if (tokens_with_numbers.empty())
    return;

  // Look for isolated numbers (e.g "[12]", "(2006)")
  for (auto token_index = tokens_with_numbers.begin();
       token_index != tokens_with_numbers.end(); ++token_index) {
    auto token = tokens_->begin() + *token_index;
    auto previous_token = GetPreviousValidToken(token);
    if (previous_token != tokens_->end() &&
        previous_token->category == kBracket) {
      auto next_token = GetNextValidToken(token);
      if (next_token != tokens_->end() &&
          next_token->category == kBracket) {
        auto number = StringToInt(token->content);
        // While there are about a dozen anime series with more than 1000
        // episodes (e.g. Doraemon), it's safe to assume that any number within
        // the interval is not the episode number.
        if (number > 1900 && number < 2050) {
          data_->anime_year = token->content;
          // We don't set token category to identifier here, because there might
          // be a good reason to keep the year as a part of the title, as in
          // "Fullmetal Alchemist (2009)".
        } else if (number <= 1900) {
          SetEpisodeNumber(token->content, *token);
          return;
        }
      }
    }
  }

  // Look for numbers with a preceding "-" separator, in reverse order
  for (auto token_index = tokens_with_numbers.rbegin();
       token_index != tokens_with_numbers.rend(); ++token_index) {
    auto token = tokens_->begin() + *token_index;
    auto previous_token = GetPreviousValidToken(token);
    if (previous_token != tokens_->end() &&
        previous_token->category == kUnknown &&
        previous_token->content == _TEXT("-")) {
      SetEpisodeNumber(token->content, *token);
      previous_token->category = kIdentifier;
      return;
    }
  }

  // Consider using the last number as a last resort
  size_t token_index = tokens_with_numbers.back();
  auto& token = tokens_->begin() + token_index;

  // Assuming that episode number always comes after the title, first token
  // cannot be what we're looking for
  if (token_index == 0)
    return;
  // An enclosed token is unlikely to be the episode number at this point
  if (token->enclosed)
    return;
  // Ignore if it's the first non-enclosed token
  bool first_nonenclosed =
      std::all_of(tokens_->begin(), tokens_->begin() + token_index,
                  [](const Token& token) { return token.enclosed; });
  if (first_nonenclosed)
    return;
  // Check if the previous token is "Season" or "Movie"
  auto previous_token = GetPreviousValidToken(token);
  if (previous_token != tokens_->end() &&
      previous_token->category == kUnknown) {
    if (IsStringEqualTo(previous_token->content, _TEXT("Season"))) {
      // We can't bail out yet; it can still be in "2nd Season 01" format
      previous_token = GetPreviousValidToken(previous_token);
      if (previous_token != tokens_->end()) {
        if (IsCountingWord(previous_token->content)) {
          data_->anime_season = previous_token->content;
        } else {
          data_->anime_season = token->content;
          return;
        }
      }
    } else if (IsStringEqualTo(previous_token->content, _TEXT("Movie"))) {
      return;
    }
  }

  // We'll use the last number after all
  SetEpisodeNumber(token->content, *token);
}

void Parser::SetEpisodeNumber(const string_t& number, Token& token) {
  data_->episode_number = number;
  TrimString(data_->episode_number);

  token.category = kIdentifier;
}

////////////////////////////////////////////////////////////////////////////////

void BuildElement(string_t& element, bool keep_delimiters,
                  const token_iterator_t& token_begin,
                  const token_iterator_t& token_end) {
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
    TrimString(element, _TEXT(" -"));
}

void Parser::SearchForAnimeTitle() {
  // Find the first non-enclosed unknown token
  auto token_begin = std::find_if(tokens_->begin(), tokens_->end(),
      [](const Token& token) {
        return !token.enclosed && token.category == kUnknown;
      });
  if (token_begin == tokens_->end())
    return;

  // Continue until an identifier is found
  auto token_end = std::find_if(token_begin, tokens_->end(),
      [](const Token& token) {
        return token.category == kIdentifier;
      });
  // If within the interval there's an open bracket without its matching pair,
  // move title_end back to the bracket
  auto last_bracket = token_end;
  bool bracket_open = false;
  for (auto token = token_begin; token != token_end; ++token) {
    if (token->category == kBracket) {
      last_bracket = token;
      bracket_open = !bracket_open;
    }
  }
  if (bracket_open)
    token_end = last_bracket;

  // Build anime title
  BuildElement(data_->anime_title, false, token_begin, token_end);
}

void Parser::SearchForReleaseGroup() {
  auto token_begin = tokens_->begin();
  auto token_end = tokens_->begin();

  do {
    // Find the first enclosed unknown token
    token_begin = std::find_if(token_end, tokens_->end(),
        [](const Token& token) {
          return token.enclosed && token.category == kUnknown;
        });
    if (token_begin == tokens_->end())
      continue;

    // Continue until a bracket or identifier is found
    token_end = std::find_if(token_begin, tokens_->end(),
        [](const Token& token) {
          return token.category == kBracket || token.category == kIdentifier;
        });
    if (token_end->category != kBracket)
      continue;

    // Ignore if it's not the first token in group
    auto previous_token = GetPreviousValidToken(token_begin);
    if (previous_token != tokens_->end() &&
        previous_token->category != kBracket) {
      continue;
    }

    // Build release group, or anime title if it wasn't found earlier
    bool title_not_found = data_->anime_title.empty();
    auto& element = title_not_found ? data_->anime_title : data_->release_group;
    BuildElement(element, !title_not_found, token_begin, token_end);
    return;

  } while (token_begin != tokens_->end());
}

void Parser::SearchForEpisodeTitle() {
  // Find the first non-enclosed unknown token
  auto token_begin = std::find_if(tokens_->begin(), tokens_->end(),
      [](const Token& token) {
        return !token.enclosed && token.category == kUnknown;
      });
  if (token_begin == tokens_->end())
    return;

  // Continue until a bracket or identifier is found
  auto token_end = std::find_if(token_begin, tokens_->end(),
      [](const Token& token) {
        return token.category == kBracket || token.category == kIdentifier;
      });

  // Build episode title
  BuildElement(data_->episode_title, false, token_begin, token_end);
}

}  // namespace anitomy