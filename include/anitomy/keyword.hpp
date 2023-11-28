#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#include "util.hpp"

namespace anitomy::detail {

enum class KeywordKind {
  AnimeSeason,
  AnimeType,
  AudioChannels,
  AudioCodec,
  AudioLanguage,
  DeviceCompatibility,
  Episode,
  EpisodeType,
  FileExtension,
  Language,
  Other,
  ReleaseGroup,
  ReleaseInformation,
  ReleaseVersion,
  Source,
  Subtitles,
  VideoCodec,
  VideoColorDepth,
  VideoFormat,
  VideoFrameRate,
  VideoProfile,
  VideoQuality,
  VideoResolution,
  Volume,
};

struct Keyword {
  enum Flags : uint8_t {
    Unidentifiable = 0x01,
    Unbounded = 0x02,
  };

  KeywordKind kind;
  uint8_t flags = 0;

  constexpr bool is_identifiable() const noexcept {
    return (flags & Unidentifiable) != Unidentifiable;
  }

  constexpr bool is_bounded() const noexcept {
    return (flags & Unbounded) != Unbounded;
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
    []() -> const std::unordered_map<std::string_view, Keyword, KeywordHash, KeywordEqual> {
  using enum KeywordKind;
  using enum Keyword::Flags;

  // clang-format off
  return {
      // Anime season
      // Usually preceded or followed by a number (e.g. `2nd Season` or `Season 2`).
      {"Season",               {AnimeSeason, Unidentifiable}},
      {"Saison",               {AnimeSeason, Unidentifiable}},

      // Anime type
      {"TV",                   {AnimeType, Unidentifiable}},
      {"Movie",                {AnimeType, Unidentifiable}},
      {"Gekijouban",           {AnimeType, Unidentifiable}},
      {"OAD",                  {AnimeType, Unidentifiable}},
      {"OAV",                  {AnimeType, Unidentifiable}},
      {"ONA",                  {AnimeType, Unidentifiable}},
      {"OVA",                  {AnimeType, Unidentifiable}},
      {"SP",                   {AnimeType, Unidentifiable}},  // e.g. "Yumeiro Patissiere SP Professional"
      {"Special",              {AnimeType, Unidentifiable}},
      {"Specials",             {AnimeType, Unidentifiable}},

      // Audio
      //
      // Channels
      {"2.0ch",                {AudioChannels, 0}},
      {"2ch",                  {AudioChannels, 0}},
      {"5.1",                  {AudioChannels, 0}},
      {"5.1ch",                {AudioChannels, 0}},
      {"7.1",                  {AudioChannels, 0}},
      {"7.1ch",                {AudioChannels, 0}},
      {"DTS",                  {AudioChannels, 0}},
      {"DTS-ES",               {AudioChannels, 0}},
      {"DTS5.1",               {AudioChannels, 0}},
      {"Dolby TrueHD",         {AudioChannels, 0}},
      {"TrueHD",               {AudioChannels, 0}},
      {"TrueHD5.1",            {AudioChannels, 0}},
      // Codec
      {"AAC",                  {AudioCodec, 0}},
      {"AACX2",                {AudioCodec, 0}},
      {"AACX3",                {AudioCodec, 0}},
      {"AACX4",                {AudioCodec, 0}},
      {"AC3",                  {AudioCodec, 0}},
      {"EAC3",                 {AudioCodec, 0}},
      {"E-AC-3",               {AudioCodec, 0}},
      {"FLAC",                 {AudioCodec, 0}},
      {"FLACX2",               {AudioCodec, 0}},
      {"FLACX3",               {AudioCodec, 0}},
      {"FLACX4",               {AudioCodec, 0}},
      {"Lossless",             {AudioCodec, 0}},
      {"MP3",                  {AudioCodec, 0}},
      {"OGG",                  {AudioCodec, 0}},
      {"Vorbis",               {AudioCodec, 0}},
      {"Atmos",                {AudioCodec, 0}},
      {"Dolby Atmos",          {AudioCodec, 0}},
      {"Opus",                 {AudioCodec, Unidentifiable}},  // e.g. "Opus.COLORs"
      // Language
      {"DualAudio",            {AudioLanguage, 0}},
      {"Dual Audio",           {AudioLanguage, 0}},
      {"Dual-Audio",           {AudioLanguage, 0}},

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
      {"Eps",                  {Episode, 0}},
      {"Episode",              {Episode, 0}},
      {"Episodes",             {Episode, 0}},
      {"Episodio",             {Episode, 0}},
      {"Episódio",             {Episode, 0}},
      {"Capitulo",             {Episode, 0}},
      {"Folge",                {Episode, 0}},

      // Episode type
      {"OP",                   {EpisodeType, Unidentifiable}},
      {"Opening",              {EpisodeType, Unidentifiable}},
      {"ED",                   {EpisodeType, Unidentifiable}},
      {"Ending",               {EpisodeType, Unidentifiable}},
      {"NCED",                 {EpisodeType, 0}},
      {"NCOP",                 {EpisodeType, 0}},
      {"Preview",              {EpisodeType, Unidentifiable}},
      {"PV",                   {EpisodeType, Unidentifiable}},

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
      {"Spanish",              {Language, 0}},
      {"ITA",                  {Language, Unidentifiable}},  // e.g. "Bokura ga Ita"
      {"JAP",                  {Language, 0}},
      {"JPN",                  {Language, 0}},
      {"PT-BR",                {Language, 0}},
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
      //
      // Blu-ray
      {"BD",                   {Source, 0}},
      {"BDRip",                {Source, 0}},
      {"BluRay",               {Source, 0}},
      {"Blu-ray",              {Source, 0}},
      // DVD
      {"DVD",                  {Source, 0}},
      {"DVD5",                 {Source, 0}},
      {"DVD9",                 {Source, 0}},
      {"DVDISO",               {Source, 0}},
      {"DVDRip",               {Source, 0}},
      {"DVD-Rip",              {Source, 0}},
      {"R2DVD",                {Source, 0}},
      {"R2J",                  {Source, 0}},
      {"R2JDVD",               {Source, 0}},
      {"R2JDVDRip",            {Source, 0}},
      // TV
      {"HDTV",                 {Source, 0}},
      {"HDTVRip",              {Source, 0}},
      {"TVRip",                {Source, 0}},
      {"TV-Rip",               {Source, 0}},
      // Web
      {"Web",                  {Source, Unidentifiable}},
      {"Webcast",              {Source, 0}},
      {"WebDL",                {Source, 0}},
      {"Web-DL",               {Source, 0}},
      {"WebRip",               {Source, 0}},
      {"AMZN",                 {Source, 0}},  // Amazon Prime
      {"CR",                   {Source, 0}},  // Crunchyroll
      {"Crunchyroll",          {Source, 0}},
      {"DSNP",                 {Source, 0}},  // Disney+
      {"Funi",                 {Source, 0}},  // Funimation
      {"Funimation",           {Source, 0}},
      {"HIDI",                 {Source, 0}},  // Hidive
      {"Hidive",               {Source, 0}},
      {"Hulu",                 {Source, 0}},
      {"Netflix",              {Source, 0}},
      {"NF",                   {Source, 0}},  // Netflix
      {"VRV",                  {Source, 0}},
      {"YouTube",              {Source, 0}},

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

      // Video
      //
      // Color depth
      {"8bit",                 {VideoColorDepth, 0}},
      {"8-bit",                {VideoColorDepth, 0}},
      {"10bit",                {VideoColorDepth, 0}},
      {"10bits",               {VideoColorDepth, 0}},
      {"10-bit",               {VideoColorDepth, 0}},
      {"10-bits",              {VideoColorDepth, 0}},
      // Codec
      {"AV1",                  {VideoCodec, 0}},
      {"AVC",                  {VideoCodec, 0}},
      {"DivX",                 {VideoCodec, 0}},  // @Warning: Duplicate
      {"DivX5",                {VideoCodec, 0}},
      {"DivX6",                {VideoCodec, 0}},
      {"H.264",                {VideoCodec, 0}},
      {"H.265",                {VideoCodec, 0}},
      {"X.264",                {VideoCodec, 0}},
      {"H264",                 {VideoCodec, 0}},
      {"H265",                 {VideoCodec, 0}},
      {"X264",                 {VideoCodec, 0}},
      {"X265",                 {VideoCodec, 0}},
      {"HEVC",                 {VideoCodec, 0}},
      {"HEVC2",                {VideoCodec, 0}},
      {"Xvid",                 {VideoCodec, 0}},
      {"HDR",                  {VideoCodec, 0}},
      {"DV",                   {VideoCodec, 0}},
      {"Dolby Vision",         {VideoCodec, 0}},
      // Format
      {"AVI",                  {VideoFormat, 0}},  // @Warning: Duplicate
      {"RMVB",                 {VideoFormat, 0}},  // @Warning: Duplicate
      {"WMV",                  {VideoFormat, 0}},  // @Warning: Duplicate
      {"WMV3",                 {VideoFormat, 0}},
      {"WMV9",                 {VideoFormat, 0}},
      // Frame rate
      {"23.976FPS",            {VideoFrameRate, 0}},
      {"24FPS",                {VideoFrameRate, 0}},
      {"29.97FPS",             {VideoFrameRate, 0}},
      {"30FPS",                {VideoFrameRate, 0}},
      {"60FPS",                {VideoFrameRate, 0}},
      {"120FPS",               {VideoFrameRate, 0}},
      // Profile
      {"Hi10",                 {VideoProfile, 0}},
      {"Hi10p",                {VideoProfile, 0}},
      {"Hi444",                {VideoProfile, 0}},
      {"Hi444P",               {VideoProfile, 0}},
      {"Hi444PP",              {VideoProfile, 0}},
      // Quality
      {"HD",                   {VideoQuality, 0}},
      {"SD",                   {VideoQuality, 0}},
      {"HQ",                   {VideoQuality, 0}},
      {"LQ",                   {VideoQuality, 0}},
      // Resolution
      {"1080p",                {VideoResolution, Unbounded}},
      {"1440p",                {VideoResolution, Unbounded}},
      {"2160p",                {VideoResolution, Unbounded}},
      {"4K",                   {VideoResolution, 0}},

      // Volume
      {"Vol",                  {Volume, 0}},
      {"Volume",               {Volume, 0}},
  };
  // clang-format on
}();

}  // namespace anitomy::detail
