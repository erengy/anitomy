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

#include "keyword.h"

namespace anitomy {

KeywordManager keyword_manager;

KeywordList::KeywordList()
    : max_length_(0),
      min_length_(static_cast<size_t>(-1)) {
}

void KeywordList::Add(const string_t& str) {
  keys_.insert(str);

  if (str.size() > max_length_)
    max_length_ = str.size();
  if (str.size() < min_length_)
    min_length_ = str.size();
}

bool KeywordList::Find(const string_t& str) const {
  if (str.size() < min_length_ || str.size() > max_length_)
    return false;

  return keys_.find(str) != keys_.end();
}

////////////////////////////////////////////////////////////////////////////////

KeywordManager::KeywordManager() {
  Add(kKeywordAudio,
      // Audio channels
      _TEXT("2CH, 5.1, 5.1CH, DTS, DTS5.1, DTS-ES, TRUEHD5.1, ")
      // Audio codecs
      _TEXT("AAC, AC3, FLAC, OGG, MP3, VORBIS, ")
      // Audio language
      _TEXT("DUALAUDIO, DUAL AUDIO")
      );

  Add(kKeywordVideo,
      // Video codecs
      _TEXT("8BIT, 8-BIT, 10BIT, 10-BIT, HI10P, ")
      _TEXT("H264, H.264, X264, X.264, ")
      _TEXT("DIVX, XVID, ")
      // Video formats
      _TEXT("AVI, RMVB, WMV, ")
      // Video quality
      _TEXT("HQ, LQ, ")
      // Video resolution
      _TEXT("HD, HDTV, SD, ")
      // Other
      _TEXT("TS, VFR")
      );

  Add(kKeywordExtra,
      // Language
      _TEXT("ENG, ENGLISH, VOSTFR, ")
      // Release
      _TEXT("BATCH, COMPLETE, ")
      // Source
      _TEXT("BD, BLURAY, BLU-RAY, ")
      _TEXT("DVD, DVD5, DVD9, DVD-R2J, DVDRIP, R2DVD, R2J, R2JDVD, ")
      _TEXT("WEBCAST, ")
      // Subtitles
      _TEXT("ASS, HARDSUB, SOFTSUB, SUB, SUBBED, ")
      // Other
      _TEXT("DIRECTOR'S CUT, UNCENSORED, UNCUT, PS3, RAW, REMASTERED, WIDESCREEN, WS")
      );

  Add(kKeywordExtraUnsafe,
      // These words can be a part of the anime/episode title
      _TEXT("END, FINAL, OAV, ONA, OVA")
      );

  Add(kKeywordVersion,
      _TEXT("V0, V1, V2, V3, V4"));

  Add(kKeywordValidExtension,
      _TEXT("3GP, AVI, DIVX, FLV, MKV, MOV, MP4, MPG, OGM, RM, RMVB, WMV")
      );

  Add(kKeywordEpisodePrefix,
      _TEXT("E, EP, EP., EPS, EPS., EPISODE, ")
      _TEXT("VOL, VOL., VOLUME, ")
#ifdef ANITOMY_USE_WIDE_CHARACTERS
      _TEXT("\x7B2C")
#endif
      );

  Add(kKeywordGroup,
      _TEXT("THORA")
      );
}

void KeywordManager::Add(KeywordCategory category, const string_t& input) {
  auto& keyword_lists = keyword_lists_[category];

  const string_t separator = _TEXT(", ");
  size_t index_begin = 0, index_end;

  do {
    index_end = input.find(separator, index_begin);
    if (index_end == string_t::npos)
      index_end = input.size();
    keyword_lists.Add(input.substr(index_begin, index_end - index_begin));
    index_begin = index_end + separator.length();
  } while (index_begin <= input.size());
}

bool KeywordManager::Find(KeywordCategory category,
                          const string_t& str) {
  return keyword_lists_[category].Find(str);
}

}  // namespace anitomy