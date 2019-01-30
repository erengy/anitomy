/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>

#include <anitomy/keyword.hpp>
#include <anitomy/token.hpp>
#include <anitomy/util.hpp>

namespace anitomy {

KeywordManager keyword_manager;

KeywordManager::KeywordManager() {
  const KeywordOptions options_default;
  const KeywordOptions options_invalid{true, true, false};
  const KeywordOptions options_unidentifiable{false, true, true};
  const KeywordOptions options_unidentifiable_invalid{false, true, false};
  const KeywordOptions options_unidentifiable_unsearchable{false, false, true};

  Add(ElementType::AnimeSeasonPrefix, options_unidentifiable, {
      L"SAISON", L"SEASON"});

  Add(ElementType::AnimeType, options_unidentifiable, {
      L"GEKIJOUBAN", L"MOVIE",
      L"OAD", L"OAV", L"ONA", L"OVA",
      L"SPECIAL", L"SPECIALS",
      L"TV"});
  Add(ElementType::AnimeType, options_unidentifiable_unsearchable, {
      L"SP"});  // e.g. "Yumeiro Patissiere SP Professional"
  Add(ElementType::AnimeType, options_unidentifiable_invalid, {
      L"ED", L"ENDING", L"NCED",
      L"NCOP", L"OP", L"OPENING",
      L"PREVIEW", L"PV"});

  Add(ElementType::AudioTerm, options_default, {
      // Audio channels
      L"2.0CH", L"2CH", L"5.1", L"5.1CH", L"DTS", L"DTS-ES", L"DTS5.1",
      L"TRUEHD5.1",
      // Audio codec
      L"AAC", L"AACX2", L"AACX3", L"AACX4", L"AC3", L"EAC3", L"E-AC-3",
      L"FLAC", L"FLACX2", L"FLACX3", L"FLACX4", L"LOSSLESS", L"MP3", L"OGG",
      L"VORBIS",
      // Audio language
      L"DUALAUDIO", L"DUAL AUDIO"});

  Add(ElementType::DeviceCompatibility, options_default, {
      L"IPAD3", L"IPHONE5", L"IPOD", L"PS3", L"XBOX", L"XBOX360"});
  Add(ElementType::DeviceCompatibility, options_unidentifiable, {
      L"ANDROID"});

  Add(ElementType::EpisodePrefix, options_default, {
      L"EP", L"EP.", L"EPS", L"EPS.", L"EPISODE", L"EPISODE.", L"EPISODES",
      L"CAPITULO", L"EPISODIO", L"FOLGE"});
  Add(ElementType::EpisodePrefix, options_invalid, {
      L"E", L"\x7B2C"});  // single-letter episode keywords are not valid tokens

  Add(ElementType::FileExtension, options_default, {
      L"3GP", L"AVI", L"DIVX", L"FLV", L"M2TS", L"MKV", L"MOV", L"MP4", L"MPG",
      L"OGM", L"RM", L"RMVB", L"TS", L"WEBM", L"WMV"});
  Add(ElementType::FileExtension, options_invalid, {
      L"AAC", L"AIFF", L"FLAC", L"M4A", L"MP3", L"MKA", L"OGG", L"WAV", L"WMA",
      L"7Z", L"RAR", L"ZIP",
      L"ASS", L"SRT"});

  Add(ElementType::Language, options_default, {
      L"ENG", L"ENGLISH", L"ESPANOL", L"JAP", L"PT-BR", L"SPANISH", L"VOSTFR"});
  Add(ElementType::Language, options_unidentifiable, {
      L"ESP", L"ITA"});  // e.g. "Tokyo ESP", "Bokura ga Ita"

  Add(ElementType::Other, options_default, {
      L"REMASTER", L"REMASTERED", L"UNCENSORED", L"UNCUT",
      L"TS", L"VFR", L"WIDESCREEN", L"WS"});

  Add(ElementType::ReleaseGroup, options_default, {
      L"THORA"});

  Add(ElementType::ReleaseInformation, options_default, {
      L"BATCH", L"COMPLETE", L"PATCH", L"REMUX"});
  Add(ElementType::ReleaseInformation, options_unidentifiable, {
      L"END", L"FINAL"});  // e.g. "The End of Evangelion", "Final Approach"

  Add(ElementType::ReleaseVersion, options_default, {
      L"V0", L"V1", L"V2", L"V3", L"V4"});

  Add(ElementType::Source, options_default, {
      L"BD", L"BDRIP", L"BLURAY", L"BLU-RAY",
      L"DVD", L"DVD5", L"DVD9", L"DVD-R2J", L"DVDRIP", L"DVD-RIP",
      L"R2DVD", L"R2J", L"R2JDVD", L"R2JDVDRIP",
      L"HDTV", L"HDTVRIP", L"TVRIP", L"TV-RIP",
      L"WEBCAST", L"WEBRIP"});

  Add(ElementType::Subtitles, options_default, {
      L"ASS", L"BIG5", L"DUB", L"DUBBED", L"HARDSUB", L"HARDSUBS", L"RAW",
      L"SOFTSUB", L"SOFTSUBS", L"SUB", L"SUBBED", L"SUBTITLED"});

  Add(ElementType::VideoTerm, options_default, {
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

  Add(ElementType::VolumePrefix, options_default, {
      L"VOL", L"VOL.", L"VOLUME"});
}

void KeywordManager::Add(ElementType type,
                         const KeywordOptions& options,
                         const std::initializer_list<string_t>& keywords) {
  auto& keys = GetKeywordContainer(type);
  for (const auto& keyword : keywords) {
    if (keyword.empty())
      continue;
    if (keys.find(keyword) != keys.end())
      continue;
    keys.insert(std::make_pair(keyword, Keyword{type, options}));
  }
}

bool KeywordManager::Find(ElementType type, const string_t& str) const {
  const auto& keys = GetKeywordContainer(type);
  auto it = keys.find(str);
  if (it != keys.end() && it->second.type == type)
    return true;

  return false;
}

bool KeywordManager::Find(const string_t& str, ElementType& type,
                          KeywordOptions& options) const {
  const auto& keys = GetKeywordContainer(type);
  auto it = keys.find(str);
  if (it != keys.end()) {
    if (type == ElementType::Unknown) {
      type = it->second.type;
    } else if (it->second.type != type) {
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
    ElementType type) const {
  return type == ElementType::FileExtension ?
      const_cast<keyword_container_t&>(file_extensions_) :
      const_cast<keyword_container_t&>(keys_);
}

}  // namespace anitomy
