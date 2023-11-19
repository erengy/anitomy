#pragma once

#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#include "util.hpp"

namespace anitomy::detail {

enum class KeywordKind {
  AnimeSeason,
  AnimeType,
  AudioTerm,
  DeviceCompatibility,
  Episode,
  FileExtension,
  Language,
  Other,
  ReleaseGroup,
  ReleaseInformation,
  ReleaseVersion,
  Source,
  Subtitles,
  VideoResolution,
  VideoTerm,
  Volume,
};

struct KeywordProps {
  enum Flags : uint8_t {
    Unidentifiable = 0x01,
    Unsearchable = 0x02,
    Unbounded = 0x04,
  };

  uint8_t value = 0;

  constexpr bool is_identifiable() const noexcept {
    return (value & Unidentifiable) != Unidentifiable;
  }

  constexpr bool is_searchable() const noexcept {
    return (value & Unsearchable) != Unsearchable;
  }

  constexpr bool is_bounded() const noexcept {
    return (value & Unbounded) != Unbounded;
  }
};

struct KeywordHash {
  [[nodiscard]] size_t operator()(std::string_view view) const noexcept {
    std::string str = view | std::views::transform([](char ch) { return to_lower(ch); }) |
                      std::ranges::to<std::string>();
    return std::hash<std::string>()(str);
  }
};

struct KeywordEqual {
  [[nodiscard]] bool operator()(std::string_view a, std::string_view b) const noexcept {
    return std::ranges::equal(a, b, [](char a, char b) { return to_lower(a) == to_lower(b); });
  }
};

inline auto keywords =
    []() -> const std::unordered_map<std::string_view, std::tuple<KeywordKind, uint8_t>,
                                     KeywordHash, KeywordEqual> {
  using enum KeywordKind;
  using enum KeywordProps::Flags;

  // clang-format off
  return {
      // Anime season
      // Usually preceded or followed by a number (e.g. `2nd Season` or `Season 2`).
      {"Season",               {AnimeSeason, Unidentifiable}},
      {"Saison",               {AnimeSeason, Unidentifiable}},

      // Anime type
      {"Movie",                {AnimeType, Unidentifiable}},
      {"Gekijouban",           {AnimeType, Unidentifiable}},
      {"OAD",                  {AnimeType, Unidentifiable}},
      {"OAV",                  {AnimeType, Unidentifiable}},
      {"ONA",                  {AnimeType, Unidentifiable}},
      {"OVA",                  {AnimeType, Unidentifiable}},
      {"SP",                   {AnimeType, Unidentifiable | Unsearchable}},  // e.g. "Yumeiro Patissiere SP Professional"
      {"Special",              {AnimeType, Unidentifiable}},
      {"Specials",             {AnimeType, Unidentifiable}},
      {"TV",                   {AnimeType, Unidentifiable}},
      {"ED",                   {AnimeType, Unidentifiable}},
      {"Ending",               {AnimeType, Unidentifiable}},
      {"NCED",                 {AnimeType, Unidentifiable}},
      {"NCOP",                 {AnimeType, Unidentifiable}},
      {"OP",                   {AnimeType, Unidentifiable}},
      {"Opening",              {AnimeType, Unidentifiable}},
      {"Preview",              {AnimeType, Unidentifiable}},
      {"PV",                   {AnimeType, Unidentifiable}},

      // Audio term
      //
      // Audio channels
      {"2.0ch",                {AudioTerm, 0}},
      {"2ch",                  {AudioTerm, 0}},
      {"5.1",                  {AudioTerm, 0}},
      {"5.1ch",                {AudioTerm, 0}},
      {"7.1",                  {AudioTerm, 0}},
      {"7.1ch",                {AudioTerm, 0}},
      {"DTS",                  {AudioTerm, 0}},
      {"DTS-ES",               {AudioTerm, 0}},
      {"DTS5.1",               {AudioTerm, 0}},
      {"Dolby TrueHD",         {AudioTerm, 0}},
      {"TrueHD",               {AudioTerm, 0}},
      {"TrueHD5.1",            {AudioTerm, 0}},
      // Audio codec
      {"AAC",                  {AudioTerm, 0}},
      {"AACX2",                {AudioTerm, 0}},
      {"AACX3",                {AudioTerm, 0}},
      {"AACX4",                {AudioTerm, 0}},
      {"AC3",                  {AudioTerm, 0}},
      {"EAC3",                 {AudioTerm, 0}},
      {"E-AC-3",               {AudioTerm, 0}},
      {"FLAC",                 {AudioTerm, 0}},
      {"FLACX2",               {AudioTerm, 0}},
      {"FLACX3",               {AudioTerm, 0}},
      {"FLACX4",               {AudioTerm, 0}},
      {"Lossless",             {AudioTerm, 0}},
      {"MP3",                  {AudioTerm, 0}},
      {"OGG",                  {AudioTerm, 0}},
      {"Vorbis",               {AudioTerm, 0}},
      {"Atmos",                {AudioTerm, 0}},
      {"Dolby Atmos",          {AudioTerm, 0}},
      {"Opus",                 {AudioTerm, Unidentifiable}},  // e.g. "Opus.COLORs"
      // Audio language
      {"DualAudio",            {AudioTerm, 0}},
      {"Dual Audio",           {AudioTerm, 0}},

      // Device compatibility
      {"Android",              {DeviceCompatibility, Unidentifiable}},  // e.g. "Dragon Ball Z: Super Android 13"
      {"iPad3",                {DeviceCompatibility, 0}},
      {"iPhone5",              {DeviceCompatibility, 0}},
      {"iPod",                 {DeviceCompatibility, 0}},
      {"PS3",                  {DeviceCompatibility, 0}},
      {"Xbox",                 {DeviceCompatibility, 0}},
      {"Xbox360",              {DeviceCompatibility, 0}},

      // Episode prefix
      {"Ep",                   {Episode, 0}},
      {"Ep.",                  {Episode, 0}},
      {"Eps",                  {Episode, 0}},
      {"Eps.",                 {Episode, 0}},
      {"Episode",              {Episode, 0}},
      {"Episode.",             {Episode, 0}},
      {"Episodes",             {Episode, 0}},
      {"Capitulo",             {Episode, 0}},
      {"Episodio",             {Episode, 0}},
      {"Episódio",             {Episode, 0}},
      {"Folge",                {Episode, 0}},

      // File extension
      {"3gp",                  {FileExtension, 0}},
      {"avi",                  {FileExtension, 0}},
      {"divx",                 {FileExtension, 0}},
      {"flv",                  {FileExtension, 0}},
      {"m2ts",                 {FileExtension, 0}},
      {"mkv",                  {FileExtension, 0}},
      {"mov",                  {FileExtension, 0}},
      {"mp4",                  {FileExtension, 0}},
      {"mpg",                  {FileExtension, 0}},
      {"ogm",                  {FileExtension, 0}},
      {"rm",                   {FileExtension, 0}},
      {"rmvb",                 {FileExtension, 0}},
      {"ts",                   {FileExtension, 0}},
      {"webm",                 {FileExtension, 0}},
      {"wmv",                  {FileExtension, 0}},

      // Language
      {"ENG",                  {Language, 0}},
      {"English",              {Language, 0}},
      {"ESP",                  {Language, Unidentifiable}},  // e.g. "Tokyo ESP"
      {"Espanol",              {Language, 0}},
      {"ITA",                  {Language, Unidentifiable}},  // e.g. "Bokura ga Ita"
      {"JAP",                  {Language, 0}},
      {"PT-BR",                {Language, 0}},
      {"Spanish",              {Language, 0}},
      {"VOSTFR",               {Language, 0}},

      // Other
      {"Remaster",             {Other, 0}},
      {"Remastered",           {Other, 0}},
      {"Uncensored",           {Other, 0}},
      {"Uncut",                {Other, 0}},
      {"TS",                   {Other, 0}},
      {"VFR",                  {Other, 0}},
      {"Widescreen",           {Other, 0}},
      {"WS",                   {Other, 0}},

      // Release group
      {"THORA",                {ReleaseGroup, 0}},  // special case because usually placed at the end

      // Release information
      {"Batch",                {ReleaseInformation, 0}},
      {"Complete",             {ReleaseInformation, 0}},
      {"End",                  {ReleaseInformation, Unidentifiable}},  // e.g. "The End of Evangelion"
      {"Final",                {ReleaseInformation, Unidentifiable}},  // e.g. "Final Approach"
      {"Patch",                {ReleaseInformation, 0}},
      {"Remux",                {ReleaseInformation, 0}},

      // Release version
      {"v0",                   {ReleaseVersion, 0}},
      {"v1",                   {ReleaseVersion, 0}},
      {"v2",                   {ReleaseVersion, 0}},
      {"v3",                   {ReleaseVersion, 0}},
      {"v4",                   {ReleaseVersion, 0}},

      // Source
      {"BD",                   {Source, 0}},
      {"BDRip",                {Source, 0}},
      {"BluRay",               {Source, 0}},
      {"Blu-Ray",              {Source, 0}},
      {"DVD",                  {Source, 0}},
      {"DVD5",                 {Source, 0}},
      {"DVD9",                 {Source, 0}},
      {"DVD-R2J",              {Source, 0}},
      {"DVDRip",               {Source, 0}},
      {"DVD-Rip",              {Source, 0}},
      {"R2DVD",                {Source, 0}},
      {"R2J",                  {Source, 0}},
      {"R2JDVD",               {Source, 0}},
      {"R2JDVDRip",            {Source, 0}},
      {"HDTV",                 {Source, 0}},
      {"HDTVRip",              {Source, 0}},
      {"TVRip",                {Source, 0}},
      {"TV-Rip",               {Source, 0}},
      {"Webcast",              {Source, 0}},
      {"WebRip",               {Source, 0}},

      // Subtitles
      {"ASS",                  {Subtitles, 0}},
      {"BIG5",                 {Subtitles, 0}},
      {"Dub",                  {Subtitles, 0}},
      {"Dubbed",               {Subtitles, 0}},
      {"Hardsub",              {Subtitles, 0}},
      {"Hardsubs",             {Subtitles, 0}},
      {"RAW",                  {Subtitles, 0}},
      {"Softsub",              {Subtitles, 0}},
      {"Softsubs",             {Subtitles, 0}},
      {"Sub",                  {Subtitles, 0}},
      {"Subbed",               {Subtitles, 0}},
      {"Subtitled",            {Subtitles, 0}},
      {"Multisub",             {Subtitles, 0}},
      {"Multi Sub",            {Subtitles, 0}},

      // Video resolution
      {"1080p",                {VideoResolution, Unbounded}},  // e.g. `BD1080p`
      {"1440p",                {VideoResolution, Unbounded}},
      {"2160p",                {VideoResolution, Unbounded}},

      // Video term
      //
      // Frame rate
      {"23.976FPS",            {VideoTerm, 0}},
      {"24FPS",                {VideoTerm, 0}},
      {"29.97FPS",             {VideoTerm, 0}},
      {"30FPS",                {VideoTerm, 0}},
      {"60FPS",                {VideoTerm, 0}},
      {"120FPS",               {VideoTerm, 0}},
      // Video codec
      {"8bit",                 {VideoTerm, 0}},
      {"8-bit",                {VideoTerm, 0}},
      {"10bit",                {VideoTerm, 0}},
      {"10bits",               {VideoTerm, 0}},
      {"10-bit",               {VideoTerm, 0}},
      {"10-bits",              {VideoTerm, 0}},
      {"Hi10",                 {VideoTerm, 0}},
      {"Hi10p",                {VideoTerm, 0}},
      {"Hi444",                {VideoTerm, 0}},
      {"Hi444P",               {VideoTerm, 0}},
      {"Hi444PP",              {VideoTerm, 0}},
      {"HDR",                  {VideoTerm, 0}},
      {"DV",                   {VideoTerm, 0}},
      {"Dolby Vision",         {VideoTerm, 0}},
      {"H264",                 {VideoTerm, 0}},
      {"H265",                 {VideoTerm, 0}},
      {"H.264",                {VideoTerm, 0}},
      {"H.265",                {VideoTerm, 0}},
      {"X264",                 {VideoTerm, 0}},
      {"X265",                 {VideoTerm, 0}},
      {"X.264",                {VideoTerm, 0}},
      {"AVC",                  {VideoTerm, 0}},
      {"HEVC",                 {VideoTerm, 0}},
      {"HEVC2",                {VideoTerm, 0}},
      {"DivX",                 {VideoTerm, 0}},
      {"DivX5",                {VideoTerm, 0}},
      {"DivX6",                {VideoTerm, 0}},
      {"Xvid",                 {VideoTerm, 0}},
      {"AV1",                  {VideoTerm, 0}},
      // Video format
      {"AVI",                  {VideoTerm, 0}},
      {"RMVB",                 {VideoTerm, 0}},
      {"WMV",                  {VideoTerm, 0}},
      {"WMV3",                 {VideoTerm, 0}},
      {"WMV9",                 {VideoTerm, 0}},
      // Video quality
      {"HQ",                   {VideoTerm, 0}},
      {"LQ",                   {VideoTerm, 0}},
      // Video resolution
      {"4K",                   {VideoTerm, 0}},
      {"HD",                   {VideoTerm, 0}},
      {"SD",                   {VideoTerm, 0}},

      // Volume
      {"Vol",                  {Volume, 0}},
      {"Vol.",                 {Volume, 0}},
      {"Volume",               {Volume, 0}},
  };
  // clang-format on
}();

}  // namespace anitomy::detail
