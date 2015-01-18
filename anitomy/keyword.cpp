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
#include "token.h"

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

bool KeywordList::Find(const string_t& str, KeywordOptions& options) const {
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

  Add(kElementAnimeType, options_unsafe, {
      L"OAV", L"ONA", L"OVA", L"TV",
      L"ED", L"ENDING", L"NCED", L"NCOP", L"OP", L"OPENING", L"PV"});

  Add(kElementAudioTerm, options_safe, {
      // Audio channels
      L"2CH", L"5.1", L"5.1CH", L"DTS", L"DTS-ES", L"DTS5.1", L"TRUEHD5.1",
      // Audio codec
      L"AAC", L"AC3", L"FLAC", L"MP3", L"OGG", L"VORBIS",
      // Audio language
      L"DUALAUDIO", L"DUAL AUDIO"});

  Add(kElementDeviceCompatibility, options_safe, {
      L"IPAD3", L"IPHONE5", L"IPOD", L"PS3", L"XBOX", L"XBOX360"});
  Add(kElementDeviceCompatibility, options_unsafe, {
      L"ANDROID"});

  Add(kElementEpisodePrefix, options_safe, {
      L"E", L"EP", L"EP.", L"EPS", L"EPS.", L"EPISODE", L"EPISODE.",
      L"VOL", L"VOL.", L"VOLUME",
      L"EPISODIO", L"FOLGE", L"\x7B2C"});

  Add(kElementFileExtension, options_safe, {
      L"3GP", L"AVI", L"DIVX", L"FLV", L"M2TS", L"MKV", L"MOV", L"MP4", L"MPG",
      L"OGM", L"RM", L"RMVB", L"WMV"});

  Add(kElementLanguage, options_safe, {
      L"ENG", L"ENGLISH", L"ESPANOL", L"JAP", L"SPANISH", L"VOSTFR"});
  Add(kElementLanguage, options_unsafe, {
      L"ESP", L"ITA"});  // e.g. "Tokyo ESP", "Bokura ga Ita"

  Add(kElementOther, options_safe, {
      L"REMASTER", L"REMASTERED", L"UNCENSORED", L"UNCUT",
      L"TS", L"VFR", L"WIDESCREEN", L"WS"});

  Add(kElementReleaseGroup, options_safe, {
      L"THORA"});

  Add(kElementReleaseInformation, options_safe, {
      L"BATCH", L"COMPLETE"});
  Add(kElementReleaseInformation, options_unsafe, {
      L"END", L"FINAL"});  // e.g. "The End of Evangelion", "Final Approach"

  Add(kElementReleaseVersion, options_safe, {
      L"V0", L"V1", L"V2", L"V3", L"V4"});

  Add(kElementSource, options_safe, {
      L"BD", L"BDRIP", L"BLURAY", L"BLU-RAY",
      L"DVD", L"DVD5", L"DVD9", L"DVD-R2J", L"DVDRIP", L"DVD-RIP",
      L"R2DVD", L"R2J", L"R2JDVD", L"R2JDVDRIP",
      L"HDTV", L"HDTVRIP", L"TVRIP", L"TV-RIP", L"WEBCAST"});

  Add(kElementSubtitles, options_safe, {
      L"ASS", L"BIG5", L"DUBBED", L"HARDSUB", L"RAW", L"SOFTSUB", L"SUB",
      L"SUBBED", L"SUBTITLED"});

  Add(kElementVideoTerm, options_safe, {
      // Video codec
      L"8BIT", L"8-BIT", L"10BIT", L"10-BIT", L"HI10P",
      L"H264", L"H.264", L"X264", L"X.264",
      L"AVC", L"DIVX", L"XVID",
      // Video format
      L"AVI", L"RMVB", L"WMV", L"WMV3", L"WMV9",
      // Video quality
      L"HQ", L"LQ",
      // Video resolution
      L"HD", L"SD"});
}

void KeywordManager::Add(ElementCategory category,
                         const KeywordOptions& options,
                         const std::initializer_list<string_t>& keywords) {
  auto& keyword_list = keyword_lists_[category];

  for (const auto& keyword : keywords)
    keyword_list.Add(keyword, options);
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

string_t KeywordManager::Normalize(const string_t& str) const {
  return StringToUpperCopy(str);
}

////////////////////////////////////////////////////////////////////////////////

void KeywordManager::Peek(const string_t& filename,
                          const TokenRange& range,
                          Elements& elements,
                          std::vector<TokenRange>& preidentified_tokens) const {
  typedef std::pair<ElementCategory, std::vector<string_t>> entry_t;
  static const std::vector<entry_t> entries{
    {kElementAudioTerm, {L"Dual Audio"}},
    {kElementVideoTerm, {L"H264", L"H.264", L"h264", L"h.264"}},
    {kElementVideoResolution, {L"480p", L"720p", L"1080p"}},
    {kElementSource, {L"Blu-Ray"}}
  };

  auto it_begin = filename.begin() + range.offset;
  auto it_end = it_begin + range.size;

  for (const auto& entry : entries) {
    for (const auto& keyword : entry.second) {
      auto it = std::search(it_begin, it_end, keyword.begin(), keyword.end());
      if (it != it_end) {
        auto offset = it - filename.begin();
        elements.insert(entry.first, keyword);
        preidentified_tokens.push_back(TokenRange(offset, keyword.size()));
      }
    }
  }
}

}  // namespace anitomy