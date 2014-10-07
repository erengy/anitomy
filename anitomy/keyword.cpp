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

KeywordOptions::KeywordOptions()
    : safe(true) {
}

KeywordOptions::KeywordOptions(bool safe)
    : safe(safe) {
}

////////////////////////////////////////////////////////////////////////////////

KeywordList::KeywordList()
    : length_min_max_(static_cast<size_t>(-1), 0) {
}

void KeywordList::Add(const string_t& str, const KeywordOptions& options) {
  if (str.empty())
    return;

  keys_.insert(std::make_pair(str, options));

  if (str.size() > length_min_max_.second)
    length_min_max_.second = str.size();
  if (str.size() < length_min_max_.first)
    length_min_max_.first = str.size();
}

bool KeywordList::Find(const string_t& str) const {
  if (str.size() < length_min_max_.first ||
      str.size() > length_min_max_.second)
    return false;

  return keys_.find(str) != keys_.end();
}

bool KeywordList::Find(const string_t& str,
                       KeywordOptions& options) const {
  if (str.size() < length_min_max_.first ||
      str.size() > length_min_max_.second)
    return false;

  auto key = keys_.find(str);

  if (key != keys_.end()) {
    options = key->second;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

KeywordManager::KeywordManager() {
  const KeywordOptions options_safe(true);
  const KeywordOptions options_unsafe(false);

  Add(kElementAnimeType, options_unsafe,
      _TEXT("ED, OAV, ONA, OP, OVA, TV"));

  Add(kElementAudioTerm, options_safe,
      // Audio channels
      _TEXT("2CH, 5.1, 5.1CH, DTS, DTS-ES, DTS5.1, TRUEHD5.1, ")
      // Audio codec
      _TEXT("AAC, AC3, FLAC, MP3, OGG, VORBIS, ")
      // Audio language
      _TEXT("DUALAUDIO, DUAL AUDIO"));

  Add(kElementDeviceCompatibility, options_unsafe,
      _TEXT("ANDROID"));
  Add(kElementDeviceCompatibility, options_safe,
      _TEXT("IPAD3, IPHONE5, IPOD, PS3, XBOX, XBOX360"));

  Add(kElementEpisodePrefix, options_safe,
      _TEXT("E, EP, EP., EPS, EPS., EPISODE, EPISODE., ")
      _TEXT("EPISODIO, FOLGE, ")
      _TEXT("VOL, VOL., VOLUME, ")
#ifdef ANITOMY_USE_WIDE_CHARACTERS
      _TEXT("\x7B2C")
#else
      // TODO
#endif
      );

  Add(kElementFileExtension, options_safe,
      _TEXT("3GP, AVI, DIVX, FLV, MKV, MOV, MP4, MPG, OGM, RM, RMVB, WMV"));

  Add(kElementLanguage, options_safe,
      _TEXT("ENG, ENGLISH, ESP, ESPANOL, ITA, JAP, SPANISH, VOSTFR"));

  Add(kElementOther, options_safe,
      _TEXT("REMASTERED, UNCENSORED, UNCUT, ")
      _TEXT("TS, VFR, WIDESCREEN, WS"));

  Add(kElementReleaseGroup, options_safe,
      _TEXT("THORA"));

  Add(kElementReleaseInformation, options_safe,
      _TEXT("BATCH, COMPLETE"));
  Add(kElementReleaseInformation, options_unsafe,
      _TEXT("END, FINAL"));

  Add(kElementReleaseVersion, options_safe,
      _TEXT("V0, V1, V2, V3, V4"));

  Add(kElementSource, options_safe,
      _TEXT("BD, BDRIP, BLURAY, BLU-RAY, ")
      _TEXT("DVD, DVD5, DVD9, DVD-R2J, DVDRIP, DVD-RIP, ")
      _TEXT("R2DVD, R2J, R2JDVD, R2JDVDRIP, ")
      _TEXT("HDTV, HDTVRIP, TVRIP, TV-RIP, WEBCAST"));

  Add(kElementSubtitles, options_safe,
      _TEXT("ASS, BIG5, HARDSUB, RAW, SOFTSUB, SUB, SUBBED"));

  Add(kElementVideoTerm, options_safe,
      // Video codec
      _TEXT("8BIT, 8-BIT, 10BIT, 10-BIT, HI10P, ")
      _TEXT("H264, H.264, X264, X.264, ")
      _TEXT("AVC, DIVX, XVID, ")
      // Video format
      _TEXT("AVI, RMVB, WMV, WMV3, WMV9, ")
      // Video quality
      _TEXT("HQ, LQ, ")
      // Video resolution
      _TEXT("HD, SD"));
}

void KeywordManager::Add(ElementCategory category,
                         const KeywordOptions& options,
                         const string_t& keywords) {
  auto& keyword_lists = keyword_lists_[category];

  const string_t separator = _TEXT(", ");
  size_t index_begin = 0, index_end;

  do {
    index_end = keywords.find(separator, index_begin);
    if (index_end == string_t::npos)
      index_end = keywords.size();
    keyword_lists.Add(
        keywords.substr(index_begin, index_end - index_begin), options);
    index_begin = index_end + separator.length();
  } while (index_begin <= keywords.size());
}

bool KeywordManager::Find(ElementCategory category, const string_t& str) const {
  const auto& keyword_list = keyword_lists_.find(category);

  if (keyword_list != keyword_lists_.end())
    return keyword_list->second.Find(str);

  return false;
}

bool KeywordManager::Find(ElementCategory category, const string_t& str,
                          KeywordOptions& options) const {
  const auto& keyword_list = keyword_lists_.find(category);

  if (keyword_list != keyword_lists_.end())
    return keyword_list->second.Find(str, options);

  return false;
}

}  // namespace anitomy