/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include "keyword.h"
#include "parser.h"
#include "string.h"

namespace anitomy {

Parser::Parser(Elements& elements, const Options& options,
               token_container_t& tokens)
    : elements_(elements),
      options_(options),
      tokens_(tokens) {
}

bool Parser::Parse() {
  SearchForKeywords();

  SearchForIsolatedNumbers();

  if (options_.parse_episode_number)
    SearchForEpisodeNumber();

  SearchForAnimeTitle();

  if (options_.parse_release_group &&
      elements_.empty(kElementReleaseGroup))
    SearchForReleaseGroup();

  if (options_.parse_episode_title &&
      !elements_.empty(kElementEpisodeNumber))
    SearchForEpisodeTitle();

  ValidateElements();

  return !elements_.empty(kElementAnimeTitle);
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForKeywords() {
  for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
    auto& token = *it;

    if (token.category != kUnknown)
      continue;

    auto word = token.content;
    TrimString(word, L" -");

    if (word.empty())
      continue;
    // Don't bother if the word is a number that cannot be CRC
    if (word.size() != 8 && IsNumericString(word))
      continue;

    // Performs better than making a case-insensitive Find
    auto keyword = keyword_manager.Normalize(word);
    ElementCategory category = kElementUnknown;
    KeywordOptions options;

    if (keyword_manager.Find(keyword, category, options)) {
      if (!options_.parse_release_group && category == kElementReleaseGroup)
        continue;
      if (!IsElementCategorySearchable(category) || !options.searchable)
        continue;
      if (IsElementCategorySingular(category) && !elements_.empty(category))
        continue;
      if (category == kElementAnimeSeasonPrefix) {
        CheckAnimeSeasonKeyword(it);
        continue;
      } else if (category == kElementEpisodePrefix) {
        if (options.valid)
          CheckExtentKeyword(kElementEpisodeNumber, it);
        continue;
      } else if (category == kElementReleaseVersion) {
        word = word.substr(1);  // number without "v"
      } else if (category == kElementVolumePrefix) {
        CheckExtentKeyword(kElementVolumeNumber, it);
        continue;
      }
    } else {
      if (elements_.empty(kElementFileChecksum) && IsCrc32(word)) {
        category = kElementFileChecksum;
      } else if (elements_.empty(kElementVideoResolution) &&
                 IsResolution(word)) {
        category = kElementVideoResolution;
      }
    }

    if (category != kElementUnknown) {
      elements_.insert(category, word);
      if (options.identifiable)
        token.category = kIdentifier;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForEpisodeNumber() {
  // List all unknown tokens that contain a number
  std::vector<size_t> tokens;
  for (size_t i = 0; i < tokens_.size(); ++i) {
    auto& token = tokens_.at(i);
    if (token.category == kUnknown)
      if (FindNumberInString(token.content) != token.content.npos)
        tokens.push_back(i);
  }
  if (tokens.empty())
    return;

  found_episode_keywords_ = !elements_.empty(kElementEpisodeNumber);

  // If a token matches a known episode pattern, it has to be the episode number
  if (SearchForEpisodePatterns(tokens))
    return;

  if (!elements_.empty(kElementEpisodeNumber))
    return;  // We have previously found an episode number via keywords

  // From now on, we're only interested in numeric tokens
  auto not_numeric_string = [&](size_t index) -> bool {
    return !IsNumericString(tokens_.at(index).content);
  };
  tokens.erase(std::remove_if(tokens.begin(), tokens.end(), not_numeric_string),
               tokens.end());

  if (tokens.empty())
    return;

  // e.g. "01 (176)", "29 (04)"
  if (SearchForEquivalentNumbers(tokens))
    return;

  // e.g. " - 08"
  if (SearchForSeparatedNumbers(tokens))
    return;

  // e.g. "[12]", "(2006)"
  if (SearchForIsolatedNumbers(tokens))
    return;

  // Consider using the last number as a last resort
  SearchForLastNumber(tokens);
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForAnimeTitle() {
  bool enclosed_title = false;

  // Find the first non-enclosed unknown token
  auto token_begin = FindToken(tokens_.begin(), tokens_.end(),
                               kFlagNotEnclosed | kFlagUnknown);

  // If that doesn't work, find the first unknown token in the second enclosed
  // group, assuming that the first one is the release group
  if (token_begin == tokens_.end()) {
    enclosed_title = true;
    token_begin = tokens_.begin();
    bool skipped_previous_group = false;
    do {
      token_begin = FindToken(token_begin, tokens_.end(), kFlagUnknown);
      if (token_begin == tokens_.end())
        break;
      // Ignore groups that are composed of non-Latin characters
      if (IsMostlyLatinString(token_begin->content))
        if (skipped_previous_group)
          break;  // Found it
      // Get the first unknown token of the next group
      token_begin = FindToken(token_begin, tokens_.end(), kFlagBracket);
      token_begin = FindToken(token_begin, tokens_.end(), kFlagUnknown);
      skipped_previous_group = true;
    } while (token_begin != tokens_.end());
  }
  if (token_begin == tokens_.end())
    return;

  // Continue until an identifier (or a bracket, if the title is enclosed)
  // is found
  auto token_end = FindToken(token_begin, tokens_.end(),
      kFlagIdentifier | (enclosed_title ? kFlagBracket : kFlagNone));

  // If within the interval there's an open bracket without its matching pair,
  // move the upper endpoint back to the bracket
  if (!enclosed_title) {
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
  }

  // If the interval ends with an enclosed group (e.g. "Anime Title [Fansub]"),
  // move the upper endpoint back to the beginning of the group. We ignore
  // parentheses in order to keep certain groups (e.g. "(TV)") intact.
  if (!enclosed_title) {
    auto token = FindPreviousToken(tokens_, token_end, kFlagNotDelimiter);
    while (CheckTokenCategory(token, kBracket) &&
           token->content.front() != ')') {
      token = FindPreviousToken(tokens_, token, kFlagBracket);
      if (token != tokens_.end()) {
        token_end = token;
        token = FindPreviousToken(tokens_, token_end, kFlagNotDelimiter);
      }
    }
  }

  // Build anime title
  BuildElement(kElementAnimeTitle, false, token_begin, token_end);
}

void Parser::SearchForReleaseGroup() {
  auto token_begin = tokens_.begin();
  auto token_end = tokens_.begin();

  do {
    // Find the first enclosed unknown token
    token_begin = FindToken(token_end, tokens_.end(),
                            kFlagEnclosed | kFlagUnknown);
    if (token_begin == tokens_.end())
      return;

    // Continue until a bracket or identifier is found
    token_end = FindToken(token_begin, tokens_.end(),
                          kFlagBracket | kFlagIdentifier);
    if (token_end == tokens_.end() || token_end->category != kBracket)
      continue;

    // Ignore if it's not the first non-delimiter token in group
    auto previous_token = FindPreviousToken(tokens_, token_begin,
                                            kFlagNotDelimiter);
    if (previous_token != tokens_.end() &&
        previous_token->category != kBracket) {
      continue;
    }

    // Build release group
    BuildElement(kElementReleaseGroup, true, token_begin, token_end);
    return;
  } while (token_begin != tokens_.end());
}

void Parser::SearchForEpisodeTitle() {
  auto token_begin = tokens_.begin();
  auto token_end = tokens_.begin();

  do {
    // Find the first non-enclosed unknown token
    token_begin = FindToken(token_end, tokens_.end(),
                            kFlagNotEnclosed | kFlagUnknown);
    if (token_begin == tokens_.end())
      return;

    // Continue until a bracket or identifier is found
    token_end = FindToken(token_begin, tokens_.end(),
                          kFlagBracket | kFlagIdentifier);

    // Ignore if it's only a dash
    if (std::distance(token_begin, token_end) <= 2 &&
        IsDashCharacter(token_begin->content)) {
      continue;
    }

    // Build episode title
    BuildElement(kElementEpisodeTitle, false, token_begin, token_end);
    return;
  } while (token_begin != tokens_.end());
}

////////////////////////////////////////////////////////////////////////////////

void Parser::SearchForIsolatedNumbers() {
  for (auto token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->category != kUnknown ||
        !IsNumericString(token->content) ||
        !IsTokenIsolated(token))
      continue;

    auto number = StringToInt(token->content);

    // Anime year
    if (number >= kAnimeYearMin && number <= kAnimeYearMax) {
      if (elements_.empty(kElementAnimeYear)) {
        elements_.insert(kElementAnimeYear, token->content);
        token->category = kIdentifier;
        continue;
      }
    }

    // Video resolution
    if (number == 480 || number == 720 || number == 1080) {
      // If these numbers are isolated, it's more likely for them to be the
      // video resolution rather than the episode number. Some fansub groups
      // use these without the "p" suffix.
      if (elements_.empty(kElementVideoResolution)) {
        elements_.insert(kElementVideoResolution, token->content);
        token->category = kIdentifier;
        continue;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Parser::ValidateElements() {
  // Validate anime type and episode title
  if (!elements_.empty(kElementAnimeType) &&
      !elements_.empty(kElementEpisodeTitle)) {
    // Here we check whether the episode title contains an anime type
    const auto episode_title = elements_.get(kElementEpisodeTitle);
    for (auto it = elements_.begin(); it != elements_.end(); ) {
      if (it->first == kElementAnimeType) {
        if (IsInString(episode_title, it->second)) {
          if (episode_title.size() == it->second.size()) {
            elements_.erase(kElementEpisodeTitle);  // invalid episode title
          } else {
            const auto keyword = keyword_manager.Normalize(it->second);
            if (keyword_manager.Find(kElementAnimeType, keyword)) {
              it = elements_.erase(it);  // invalid anime type
              continue;
            }
          }
        }
      }
      ++it;
    }
  }
}

}  // namespace anitomy
