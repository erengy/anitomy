/*
** Copyright (c) 2014-2018, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include "keyword.h"
#include "token.h"

namespace anitomy {

KeywordManager keyword_manager;

KeywordManager::KeywordManager() {
  const KeywordOptions options_default;
  const KeywordOptions options_invalid{true, true, false};
  const KeywordOptions options_unidentifiable{false, true, true};
  const KeywordOptions options_unidentifiable_invalid{false, true, false};
  const KeywordOptions options_unidentifiable_unsearchable{false, false, true};

  Add(kElementAnimeSeasonPrefix, options_unidentifiable, {
      L"SAISON", L"SEASON"});

  Add(kElementAnimeType, options_unidentifiable, {
      L"GEKIJOUBAN", L"MOVIE",
      L"OAD", L"OAV", L"ONA", L"OVA",
      L"SPECIAL", L"SPECIALS",
      L"TV"});
  Add(kElementAnimeType, options_unidentifiable_unsearchable, {
      L"SP"});  // e.g. "Yumeiro Patissiere SP Professional"
  Add(kElementAnimeType, options_unidentifiable_invalid, {
      L"ED", L"ENDING", L"NCED",
      L"NCOP", L"OP", L"OPENING",
      L"PREVIEW", L"PV"});

  Add(kElementAudioTerm, options_default, {
      // Audio channels
      L"2.0CH", L"2CH", L"5.1", L"5.1CH", L"DTS", L"DTS-ES", L"DTS5.1",
      L"TRUEHD5.1",
      // Audio codec
      L"AAC", L"AACX2", L"AACX3", L"AACX4", L"AC3", L"EAC3", L"E-AC-3",
      L"FLAC", L"FLACX2", L"FLACX3", L"FLACX4", L"LOSSLESS", L"MP3", L"OGG",
      L"VORBIS",
      // Audio language
      L"DUALAUDIO", L"DUAL AUDIO"});

  Add(kElementDeviceCompatibility, options_default, {
      L"IPAD3", L"IPHONE5", L"IPOD", L"PS3", L"XBOX", L"XBOX360"});
  Add(kElementDeviceCompatibility, options_unidentifiable, {
      L"ANDROID"});

  Add(kElementEpisodePrefix, options_default, {
      L"EP", L"EP.", L"EPS", L"EPS.", L"EPISODE", L"EPISODE.", L"EPISODES",
      L"CAPITULO", L"EPISODIO", L"FOLGE"});
  Add(kElementEpisodePrefix, options_invalid, {
      L"E", L"\x7B2C"});  // single-letter episode keywords are not valid tokens

  Add(kElementFileExtension, options_default, {
      L"3GP", L"AVI", L"DIVX", L"FLV", L"M2TS", L"MKV", L"MOV", L"MP4", L"MPG",
      L"OGM", L"RM", L"RMVB", L"TS", L"WEBM", L"WMV"});
  Add(kElementFileExtension, options_invalid, {
      L"AAC", L"AIFF", L"FLAC", L"M4A", L"MP3", L"MKA", L"OGG", L"WAV", L"WMA",
      L"7Z", L"RAR", L"ZIP",
      L"ASS", L"SRT"});

  Add(kElementLanguage, options_default, {
      L"ENG", L"ENGLISH", L"ESPANOL", L"JAP", L"PT-BR", L"SPANISH", L"VOSTFR"});
  Add(kElementLanguage, options_unidentifiable, {
      L"ESP", L"ITA"});  // e.g. "Tokyo ESP", "Bokura ga Ita"

  Add(kElementOther, options_default, {
      L"REMASTER", L"REMASTERED", L"UNCENSORED", L"UNCUT",
      L"TS", L"VFR", L"WIDESCREEN", L"WS"});

  Add(kElementReleaseGroup, options_default, {
      L"THORA"});

  Add(kElementReleaseInformation, options_default, {
      L"BATCH", L"COMPLETE", L"PATCH", L"REMUX"});
  Add(kElementReleaseInformation, options_unidentifiable, {
      L"END", L"FINAL"});  // e.g. "The End of Evangelion", "Final Approach"

  Add(kElementReleaseVersion, options_default, {
      L"V0", L"V1", L"V2", L"V3", L"V4"});

  Add(kElementSource, options_default, {
      L"BD", L"BDRIP", L"BLURAY", L"BLU-RAY",
      L"DVD", L"DVD5", L"DVD9", L"DVD-R2J", L"DVDRIP", L"DVD-RIP",
      L"R2DVD", L"R2J", L"R2JDVD", L"R2JDVDRIP",
      L"HDTV", L"HDTVRIP", L"TVRIP", L"TV-RIP",
      L"WEBCAST", L"WEBRIP"});

  Add(kElementSubtitles, options_default, {
      L"ASS", L"BIG5", L"DUB", L"DUBBED", L"HARDSUB", L"HARDSUBS", L"RAW",
      L"SOFTSUB", L"SOFTSUBS", L"SUB", L"SUBBED", L"SUBTITLED"});

  Add(kElementVideoTerm, options_default, {
      // Frame rate
      L"23.976FPS", L"24FPS", L"29.97FPS", L"30FPS", L"60FPS", L"120FPS",
      // Video codec
      L"8BIT", L"8-BIT", L"10BIT", L"10BITS", L"10-BIT", L"10-BITS",
      L"HI10", L"HI10P", L"HI444", L"HI444P", L"HI444PP",
      L"H264", L"H265", L"H.264", L"H.265", L"X264", L"X265", L"X.264",
      L"AVC", L"HEVC", L"HEVC2", L"DIVX", L"DIVX5", L"DIVX6", L"XVID",
      // Video format
      L"AVI", L"RMVB", L"WMV", L"WMV3", L"WMV9",
      // Video quality
      L"HQ", L"LQ",
      // Video resolution
      L"HD", L"SD"});

  Add(kElementVolumePrefix, options_default, {
      L"VOL", L"VOL.", L"VOLUME"});
}

void KeywordManager::Add(ElementCategory category,
                         const KeywordOptions& options,
                         const std::initializer_list<string_t>& keywords) {
  auto& keys = GetKeywordContainer(category);
  for (const auto& keyword : keywords) {
    if (keyword.empty())
      continue;
    if (keys.find(keyword) != keys.end())
      continue;
    keys.insert(std::make_pair(keyword, Keyword{category, options}));
  }
}

bool KeywordManager::Find(ElementCategory category, const string_t& str) const {
  const auto& keys = GetKeywordContainer(category);
  auto it = keys.find(str);
  if (it != keys.end() && it->second.category == category)
    return true;

  return false;
}

bool KeywordManager::Find(const string_t& str, ElementCategory& category,
                          KeywordOptions& options) const {
  const auto& keys = GetKeywordContainer(category);
  auto it = keys.find(str);
  if (it != keys.end()) {
    if (category == kElementUnknown) {
      category = it->second.category;
    } else if (it->second.category != category) {
      return false;
    }
    options = it->second.options;
    return true;
  }

  return false;
}

string_t KeywordManager::Normalize(const string_t& str) const {
  return StringToUpperCopy(str);
}

KeywordManager::keyword_container_t& KeywordManager::GetKeywordContainer(
    ElementCategory category) const {
  return category == kElementFileExtension ?
      const_cast<keyword_container_t&>(file_extensions_) :
      const_cast<keyword_container_t&>(keys_);
}

////////////////////////////////////////////////////////////////////////////////

void KeywordManager::Peek(const string_t& filename,
                          const TokenRange& range,
                          Elements& elements,
                          std::vector<TokenRange>& preidentified_tokens) const {
  using entry_t = std::pair<ElementCategory, std::vector<string_t>>;
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
        const auto offset = static_cast<size_t>(std::distance(filename.begin(), it));
        elements.insert(entry.first, keyword);
        preidentified_tokens.push_back(TokenRange{offset, keyword.size()});
      }
    }
  }
}

}  // namespace anitomy
