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

#ifndef ANITOMY_PARSER_H
#define ANITOMY_PARSER_H

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

  bool SearchForEpisodePatterns(std::vector<size_t>& tokens);
  bool SearchForEquivalentNumbers(std::vector<size_t>& tokens);
  bool SearchForIsolatedNumbers(std::vector<size_t>& tokens);
  bool SearchForSeparatedNumbers(std::vector<size_t>& tokens);
  bool SearchForLastNumber(std::vector<size_t>& tokens);

  bool NumberComesAfterEpisodePrefix(Token& token);
  bool NumberComesBeforeTotalNumber(const token_iterator_t token);

  bool MatchEpisodePatterns(string_t word, Token& token);
  bool MatchSingleEpisodePattern(const string_t& word, Token& token);
  bool MatchMultiEpisodePattern(const string_t& word, Token& token);
  bool MatchSeasonAndEpisodePattern(const string_t& word, Token& token);
  bool MatchTypeAndEpisodePattern(const string_t& word, Token& token);
  bool MatchFractionalEpisodePattern(const string_t& word, Token& token);
  bool MatchPartialEpisodePattern(const string_t& word, Token& token);
  bool MatchNumberSignPattern(const string_t& word, Token& token);
  bool MatchJapaneseCounterPattern(const string_t& word, Token& token);

  bool IsValidEpisodeNumber(const string_t& number);
  bool SetEpisodeNumber(const string_t& number, Token& token, bool validate);

  size_t FindNumberInString(const string_t& str);
  string_t GetNumberFromOrdinal(const string_t& word);
  bool IsCrc32(const string_t& str);
  bool IsDashCharacter(const string_t& str);
  bool IsResolution(const string_t& str);
  bool IsElementCategorySearchable(ElementCategory category);
  bool IsElementCategorySingular(ElementCategory category);

  bool CheckAnimeSeasonKeyword(const token_iterator_t token);
  bool CheckEpisodeKeyword(const token_iterator_t token);

  void BuildElement(ElementCategory category, bool keep_delimiters,
                    const token_iterator_t token_begin,
                    const token_iterator_t token_end) const;

  bool IsTokenIsolated(const token_iterator_t token) const;

  const int kAnimeYearMin = 1900;
  const int kAnimeYearMax = 2050;
  const int kEpisodeNumberMax = kAnimeYearMin - 1;

  Elements& elements_;
  const Options& options_;
  token_container_t& tokens_;
};

}  // namespace anitomy

#endif  // ANITOMY_PARSER_H