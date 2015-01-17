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
#include "string.h"
#include "token.h"

namespace anitomy {

struct ParseOptions {
  ParseOptions();

  bool parse_episode_number;
  bool parse_episode_title;
  bool parse_release_group;
};

class Parser {
public:
  Parser(Elements& elements, token_container_t& tokens);

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  bool Parse();

  ParseOptions parse_options;

private:
  void SearchForKeywords();
  void SearchForEpisodeNumber();
  void SearchForAnimeTitle();
  void SearchForReleaseGroup();
  void SearchForEpisodeTitle();

  bool SearchForEpisodePatterns(std::vector<size_t>& tokens);
  bool SearchForIsolatedNumbers(std::vector<size_t>& tokens);
  bool SearchForSeparatedNumbers(std::vector<size_t>& tokens);
  bool SearchForLastNumber(std::vector<size_t>& tokens);

  bool NumberComesAfterEpisodePrefix(Token& token);
  bool NumberComesAfterEpisodeKeyword(const token_iterator_t& token);
  bool NumberComesBeforeTotalNumber(const token_iterator_t& token);

  bool MatchEpisodePatterns(const string_t& word, Token& token);
  bool MatchSingleEpisodePattern(const string_t& word, Token& token);
  bool MatchMultiEpisodePattern(const string_t& word, Token& token);
  bool MatchSeasonAndEpisodePattern(const string_t& word, Token& token);
  bool MatchTypeAndEpisodePattern(const string_t& word, Token& token);
  bool MatchJapaneseCounterPattern(const string_t& word, Token& token);

  void SetEpisodeNumber(string_t number, Token& token);

  size_t FindNumberInString(const string_t& str);
  bool IsCrc32(const string_t& str);
  bool IsDashCharacter(const string_t& str);
  bool IsOrdinalNumber(const string_t& word);
  bool IsResolution(const string_t& str);
  bool IsElementCategorySearchable(ElementCategory category);
  bool IsElementCategorySingular(ElementCategory category);

  void BuildElement(ElementCategory category, bool keep_delimiters,
                    const token_iterator_t& token_begin,
                    const token_iterator_t& token_end) const;

  Elements& elements_;
  token_container_t& tokens_;
};

}  // namespace anitomy

#endif  // ANITOMY_PARSER_H