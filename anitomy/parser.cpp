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

#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

Parser::Parser(Elements& data, token_container_t& tokens)
    : data_(&data),
      tokens_(&tokens) {
}

bool Parser::Parse() {
  SearchForKeywords();

  if (parse_options.parse_episode_number)
    SearchForEpisodeNumber();

  SearchForAnimeTitle();

  if (parse_options.parse_release_group &&
      data_->release_group.empty())
    SearchForReleaseGroup();

  if (parse_options.parse_episode_title)
    SearchForEpisodeTitle();

  return !data_->anime_title.empty();
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForKeywords() {
  for (auto token = tokens_->begin(); token != tokens_->end(); ++token) {
    if (token->category != kUnknown)
      continue;

    auto word = token->content;
    TrimString(word);

    // Don't bother if the word is a number that cannot be CRC
    if (word.size() != 8 && IsNumericString(word))
      continue;

    // Checksum
    if (data_->checksum.empty() && IsCrc32(word)) {
      data_->checksum = word;
      token->category = kIdentifier;
      continue;

    // Video resolution
    } else if (data_->resolution.empty() && IsResolution(word)) {
      data_->resolution = word;
      token->category = kIdentifier;
      continue;
    }

    // Performs better than making a case-insensitive Find
    auto keyword = StringToUpperCopy(word);

    // Video info
    if (keyword_manager.Find(kKeywordVideo, keyword)) {
      AppendKeyword(data_->video, word);
      token->category = kIdentifier;

    // Audio info
    } else if (keyword_manager.Find(kKeywordAudio, keyword)) {
      AppendKeyword(data_->audio, word);
      token->category = kIdentifier;

    // Version
    } else if (data_->release_version.empty() &&
               keyword_manager.Find(kKeywordVersion, keyword)) {
      data_->release_version.push_back(word.back());  // number without "v"
      token->category = kIdentifier;

    // Group
    } else if (parse_options.parse_release_group &&
               data_->release_group.empty() &&
               keyword_manager.Find(kKeywordGroup, keyword)) {
      data_->release_group = word;
      token->category = kIdentifier;

    // Extras
    } else if (parse_options.parse_extra_keywords &&
               keyword_manager.Find(kKeywordExtra, keyword)) {
      AppendKeyword(data_->extras, word);
      token->category = kIdentifier;
    } else if (parse_options.parse_extra_keywords &&
               keyword_manager.Find(kKeywordExtraUnsafe, keyword)) {
      AppendKeyword(data_->extras, word);
      if (token->enclosed)
        token->category = kIdentifier;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForEpisodeNumber() {
  // List all tokens that contain a number
  std::vector<size_t> tokens;
  for (size_t i = 0; i < tokens_->size(); i++) {
    auto& token = tokens_->at(i);
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
  for (auto it = tokens.begin(); it != tokens.end(); ) {
    if (!IsNumericString(tokens_->at(*it).content)) {
      it = tokens.erase(it);
    } else {
      ++it;
    }
  }
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
    if (data_->release_group.empty()) {
      BuildElement(data_->release_group, true, token_begin, token_end);
      if (data_->anime_title.empty())
        continue;
    } else if (data_->anime_title.empty()) {
      BuildElement(data_->anime_title, false, token_begin, token_end);
      return;
    }

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