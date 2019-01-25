/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "element.h"
#include "options.h"
#include "string.h"
#include "token.h"

namespace anitomy {

class Parser {
public:
  Parser(Elements& elements, const Options& options, token_container_t& tokens);

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  bool Parse();

private:
  void SearchForKeywords();
  void SearchForEpisodeNumber();
  void SearchForAnimeTitle();
  void SearchForReleaseGroup();
  void SearchForEpisodeTitle();
  void SearchForIsolatedNumbers();
  void ValidateElements();

  bool SearchForEpisodePatterns(std::vector<size_t>& tokens);
  bool SearchForEquivalentNumbers(std::vector<size_t>& tokens);
  bool SearchForIsolatedNumbers(std::vector<size_t>& tokens);
  bool SearchForSeparatedNumbers(std::vector<size_t>& tokens);
  bool SearchForLastNumber(std::vector<size_t>& tokens);

  bool NumberComesAfterPrefix(ElementCategory category, Token& token);
  bool NumberComesBeforeAnotherNumber(const token_iterator_t token);

  bool MatchEpisodePatterns(string_t word, Token& token);
  bool MatchSingleEpisodePattern(const string_t& word, Token& token);
  bool MatchMultiEpisodePattern(const string_t& word, Token& token);
  bool MatchSeasonAndEpisodePattern(const string_t& word, Token& token);
  bool MatchTypeAndEpisodePattern(const string_t& word, Token& token);
  bool MatchFractionalEpisodePattern(const string_t& word, Token& token);
  bool MatchPartialEpisodePattern(const string_t& word, Token& token);
  bool MatchNumberSignPattern(const string_t& word, Token& token);
  bool MatchJapaneseCounterPattern(const string_t& word, Token& token);

  bool MatchVolumePatterns(string_t word, Token& token);
  bool MatchSingleVolumePattern(const string_t& word, Token& token);
  bool MatchMultiVolumePattern(const string_t& word, Token& token);

  bool IsValidEpisodeNumber(const string_t& number);
  bool SetEpisodeNumber(const string_t& number, Token& token, bool validate);
  bool SetAlternativeEpisodeNumber(const string_t& number, Token& token);

  bool IsValidVolumeNumber(const string_t& number);
  bool SetVolumeNumber(const string_t& number, Token& token, bool validate);

  size_t FindNumberInString(const string_t& str);
  string_t GetNumberFromOrdinal(const string_t& word);
  bool IsCrc32(const string_t& str);
  bool IsDashCharacter(const string_t& str);
  bool IsResolution(const string_t& str);
  bool IsElementCategorySearchable(ElementCategory category);
  bool IsElementCategorySingular(ElementCategory category);

  bool CheckAnimeSeasonKeyword(const token_iterator_t token);
  bool CheckExtentKeyword(ElementCategory category,
                          const token_iterator_t token);

  void BuildElement(ElementCategory category, bool keep_delimiters,
                    const token_iterator_t token_begin,
                    const token_iterator_t token_end) const;

  bool CheckTokenCategory(const token_iterator_t token,
                          TokenCategory category) const;
  bool IsTokenIsolated(const token_iterator_t token) const;

  const int kAnimeYearMin = 1900;
  const int kAnimeYearMax = 2050;
  const int kEpisodeNumberMax = kAnimeYearMin - 1;
  const int kVolumeNumberMax = 20;

  Elements& elements_;
  const Options& options_;
  token_container_t& tokens_;

  bool found_episode_keywords_ = false;
};

}  // namespace anitomy
