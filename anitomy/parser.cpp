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

#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

Parser::Parser(Elements& elements, Options& options, token_container_t& tokens)
    : elements_(elements),
      options_(options),
      tokens_(tokens) {
}

bool Parser::Parse() {
  SearchForKeywords();

  SearchForAnimeSeason();
  SearchForAnimeYear();

  if (options_.parse_episode_number)
    SearchForEpisodeNumber();

  SearchForAnimeTitle();

  if (options_.parse_release_group &&
      elements_.empty(kElementReleaseGroup))
    SearchForReleaseGroup();

  if (options_.parse_episode_title)
    SearchForEpisodeTitle();

  return !elements_.empty(kElementAnimeTitle);
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForKeywords() {
  for (auto& token : tokens_) {
    if (token.category != kUnknown)
      continue;

    auto word = token.content;
    TrimString(word, L" -");

    // Don't bother if the word is a number that cannot be CRC
    if (word.size() != 8 && IsNumericString(word))
      continue;

    // Performs better than making a case-insensitive Find
    auto keyword = keyword_manager.Normalize(word);

    for (int i = kElementIterateFirst; i < kElementIterateLast; i++) {
      auto category = static_cast<ElementCategory>(i);

      if (!options_.parse_release_group)
        if (category == kElementReleaseGroup)
          continue;
      if (!IsElementCategorySearchable(category))
        continue;
      if (IsElementCategorySingular(category))
        if (!elements_.empty(category))
          continue;

      bool add_keyword = false;
      KeywordOptions options;

      switch (category) {
        case kElementFileChecksum:
          add_keyword = IsCrc32(word);
          break;
        case kElementVideoResolution:
          add_keyword = IsResolution(word);
          break;
        default:
          add_keyword = keyword_manager.Find(category, keyword, options);
          break;
      }

      if (add_keyword) {
        switch (category) {
          case kElementReleaseVersion:
            elements_.insert(category, word.substr(1));  // number without "v"
            break;
          default:
            elements_.insert(category, word);
            break;
        }
        if (options.safe || token.enclosed)
          token.category = kIdentifier;
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForEpisodeNumber() {
  // List all tokens that contain a number
  std::vector<size_t> tokens;
  for (size_t i = 0; i < tokens_.size(); ++i) {
    auto& token = tokens_.at(i);
    if (token.category != kUnknown)
      continue;  // Skip previously identified tokens
    if (FindNumberInString(token.content) != token.content.npos)
      tokens.push_back(i);
  }
  if (tokens.empty())
    return;

  // If a token matches a known episode pattern, it has to be the episode number
  if (SearchForEpisodePatterns(tokens))
    return;

  // From now on, we're only interested in numeric tokens
  auto not_numeric_string = [&](size_t index) -> bool {
    return !IsNumericString(tokens_.at(index).content);
  };
  tokens.erase(std::remove_if(tokens.begin(), tokens.end(), not_numeric_string),
               tokens.end());

  if (tokens.empty())
    return;

  // e.g. "[12]", "(2006)"
  if (SearchForIsolatedNumbers(tokens))
    return;

  // e.g. " - 08"
  if (SearchForSeparatedNumbers(tokens))
    return;

  // Consider using the last number as a last resort
  SearchForLastNumber(tokens);
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForAnimeTitle() {
  // Find the first non-enclosed unknown token
  auto token_begin = std::find_if(tokens_.begin(), tokens_.end(),
      [](const Token& token) {
        return !token.enclosed && token.category == kUnknown;
      });
  if (token_begin == tokens_.end())
    return;

  // Continue until an identifier is found
  auto token_end = std::find_if(token_begin, tokens_.end(),
      [](const Token& token) {
        return token.category == kIdentifier;
      });
  // If within the interval there's an open bracket without its matching pair,
  // move the upper endpoint back to the bracket
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
  BuildElement(kElementAnimeTitle, false, token_begin, token_end);
}

void Parser::SearchForReleaseGroup() {
  auto token_begin = tokens_.begin();
  auto token_end = tokens_.begin();

  do {
    // Find the first enclosed unknown token
    token_begin = std::find_if(token_end, tokens_.end(),
        [](const Token& token) {
          return token.enclosed && token.category == kUnknown;
        });
    if (token_begin == tokens_.end())
      continue;

    // Continue until a bracket or identifier is found
    token_end = std::find_if(token_begin, tokens_.end(),
        [](const Token& token) {
          return token.category == kBracket || token.category == kIdentifier;
        });
    if (token_end->category != kBracket)
      continue;

    // Ignore if it's not the first token in group
    auto previous_token = GetPreviousNonDelimiterToken(tokens_, token_begin);
    if (previous_token != tokens_.end() &&
        previous_token->category != kBracket) {
      continue;
    }

    // Build release group, or anime title if it wasn't found earlier
    if (elements_.empty(kElementReleaseGroup)) {
      BuildElement(kElementReleaseGroup, true, token_begin, token_end);
      if (elements_.empty(kElementAnimeTitle))
        continue;
    } else if (elements_.empty(kElementAnimeTitle)) {
      BuildElement(kElementAnimeTitle, false, token_begin, token_end);
      return;
    }

  } while (token_begin != tokens_.end());
}

void Parser::SearchForEpisodeTitle() {
  // Find the first non-enclosed unknown token
  auto token_begin = std::find_if(tokens_.begin(), tokens_.end(),
      [](const Token& token) {
        return !token.enclosed && token.category == kUnknown;
      });
  if (token_begin == tokens_.end())
    return;

  // Continue until a bracket or identifier is found
  auto token_end = std::find_if(token_begin, tokens_.end(),
      [](const Token& token) {
        return token.category == kBracket || token.category == kIdentifier;
      });

  // Build episode title
  BuildElement(kElementEpisodeTitle, false, token_begin, token_end);
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForAnimeSeason() {
  for (auto token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->category != kUnknown ||
        !IsStringEqualTo(token->content, L"Season"))
      continue;

    auto previous_token = GetPreviousNonDelimiterToken(tokens_, token);
    if (previous_token != tokens_.end()) {
      if (IsOrdinalNumber(previous_token->content)) {
        elements_.insert(kElementAnimeSeason, previous_token->content);
        previous_token->category = kIdentifier;
        token->category = kIdentifier;
        return;
      }
    }

    auto next_token = GetNextNonDelimiterToken(tokens_, token);
    if (next_token != tokens_.end()) {
      if (IsNumericString(next_token->content)) {
        elements_.insert(kElementAnimeSeason, next_token->content);
        next_token->category = kIdentifier;
        token->category = kIdentifier;
        return;
      }
    }
  }
}

void Parser::SearchForAnimeYear() {
  auto is_bracket_token = [&](token_iterator_t token) {
    return token != tokens_.end() && token->category == kBracket;
  };

  for (auto token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->category != kUnknown || !IsNumericString(token->content))
      continue;

    auto previous_token = GetPreviousNonDelimiterToken(tokens_, token);
    if (!is_bracket_token(previous_token))
      continue;
    auto next_token = GetNextNonDelimiterToken(tokens_, token);
    if (!is_bracket_token(next_token))
      continue;

    auto number = StringToInt(token->content);
    if (number >= kAnimeYearMin && number <= kAnimeYearMax) {
      elements_.insert(kElementAnimeYear, token->content);
      token->category = kIdentifier;
      return;
    }
  }
}

}  // namespace anitomy