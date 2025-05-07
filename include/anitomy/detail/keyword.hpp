#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#include <anitomy/detail/util.hpp>

namespace anitomy::detail {

enum class KeywordKind {
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
  Season,
  Source,
  Subtitles,
  Type,
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
    Ambiguous = 0x01,
    Unbounded = 0x02,
  };

  KeywordKind kind;
  uint8_t flags = 0;

  constexpr bool is_ambiguous() const noexcept {
    return (flags & Ambiguous) == Ambiguous;
  }

  constexpr bool is_bounded() const noexcept {
    return (flags & Unbounded) != Unbounded;
  }
};

struct KeywordHash {
  [[nodiscard]] size_t operator()(std::string_view view) const noexcept {
    auto str = view | std::views::transform(to_lower<char>) | std::ranges::to<std::string>();
    return std::hash<std::string>()(str);
  }
};

struct KeywordEqual {
  [[nodiscard]] bool operator()(std::string_view a, std::string_view b) const noexcept {
    return std::ranges::equal(a, b, equal_to);
  }
};

using keyword_map_t = std::unordered_map<std::string_view, Keyword, KeywordHash, KeywordEqual>;

inline auto keywords = []() -> keyword_map_t {
  using enum KeywordKind;
  using enum Keyword::Flags;

  // clang-format off
  return {
      // Audio
      //
      // Channels
      {"2.0ch",                {AudioChannels}},
      {"2ch",                  {AudioChannels}},
      {"5.1",                  {AudioChannels}},
      {"5.1ch",                {AudioChannels}},
      {"7.1",                  {AudioChannels}},
      {"7.1ch",                {AudioChannels}},
      {"DTS",                  {AudioChannels}},
      {"DTS-ES",               {AudioChannels}},
      {"DTS5.1",               {AudioChannels}},
      {"Dolby TrueHD",         {AudioChannels}},
      {"TrueHD",               {AudioChannels}},
      {"TrueHD5.1",            {AudioChannels}},
      // Codec
      {"AAC",                  {AudioCodec}},
      {"AACX2",                {AudioCodec}},
      {"AACX3",                {AudioCodec}},
      {"AACX4",                {AudioCodec}},
      {"AC3",                  {AudioCodec}},
      {"EAC3",                 {AudioCodec}},
      {"E-AC-3",               {AudioCodec}},
      {"E-AC3",                {AudioCodec}},
      {"FLAC",                 {AudioCodec}},
      {"FLACX2",               {AudioCodec}},
      {"FLACX3",               {AudioCodec}},
      {"FLACX4",               {AudioCodec}},
      {"Lossless",             {AudioCodec}},
      {"MP3",                  {AudioCodec}},
      {"OGG",                  {AudioCodec}},
      {"Vorbis",               {AudioCodec}},
      {"Atmos",                {AudioCodec}},
      {"Dolby Atmos",          {AudioCodec}},
      {"Opus",                 {AudioCodec, Ambiguous}},  // e.g. "Opus.COLORs"
      // Language
      {"DualAudio",            {AudioLanguage}},
      {"Dual Audio",           {AudioLanguage}},
      {"Dual-Audio",           {AudioLanguage}},
      {"MultiAudio",           {AudioLanguage}},
      {"Multi Audio",          {AudioLanguage}},
      {"Multi-Audio",          {AudioLanguage}},
      {"EngDub",               {AudioLanguage}},
      {"JapDub",               {AudioLanguage}},

      // Device compatibility
      {"Android",              {DeviceCompatibility, Ambiguous}},  // e.g. "Dragon Ball Z: Super Android 13"
      {"iPad3",                {DeviceCompatibility}},
      {"iPhone5",              {DeviceCompatibility}},
      {"iPod",                 {DeviceCompatibility}},
      {"PS3",                  {DeviceCompatibility}},
      {"Xbox",                 {DeviceCompatibility}},
      {"Xbox360",              {DeviceCompatibility}},

      // Episode prefix
      {"Ep",                   {Episode}},
      {"Eps",                  {Episode}},
      {"Episode",              {Episode}},
      {"Episodes",             {Episode}},
      {"Episodio",             {Episode}},
      {"Episódio",             {Episode}},
      {"Capitulo",             {Episode}},
      {"Folge",                {Episode}},

      // Episode type
      {"OP",                   {EpisodeType, Ambiguous}},  // e.g. "takt op.Destiny", "My Unique Skill Makes Me OP even at Level 1"
      {"Opening",              {EpisodeType, Ambiguous}},  // e.g. "Pool Opening"
      {"ED",                   {EpisodeType, Ambiguous}},
      {"Ending",               {EpisodeType, Ambiguous}},
      {"NCED",                 {EpisodeType}},
      {"NCOP",                 {EpisodeType}},
      {"Preview",              {EpisodeType, Ambiguous}},
      {"PV",                   {EpisodeType, Ambiguous}},

      // File extension
      {"3gp",                  {FileExtension}},
      {"avi",                  {FileExtension}},
      {"divx",                 {FileExtension}},
      {"flv",                  {FileExtension}},
      {"m2ts",                 {FileExtension}},
      {"mkv",                  {FileExtension}},
      {"mov",                  {FileExtension}},
      {"mp4",                  {FileExtension}},
      {"mpg",                  {FileExtension}},
      {"ogm",                  {FileExtension}},
      {"rm",                   {FileExtension}},
      {"rmvb",                 {FileExtension}},
      {"ts",                   {FileExtension}},
      {"webm",                 {FileExtension}},
      {"wmv",                  {FileExtension}},

      // Language
      {"CHS",                  {Language}},  // Chinese Simplified
      {"CHT",                  {Language}},  // Chinese Traditional
      {"ENG",                  {Language}},
      {"English",              {Language}},
      {"ESP",                  {Language, Ambiguous}},  // e.g. "Tokyo ESP"
      {"Espanol",              {Language}},
      {"Spanish",              {Language}},
      {"ITA",                  {Language, Ambiguous}},  // e.g. "Bokura ga Ita"
      {"JAP",                  {Language}},
      {"JPN",                  {Language}},
      {"PT-BR",                {Language}},
      {"VOSTFR",               {Language}},

      // Other
      {"Remaster",             {Other}},
      {"Remastered",           {Other}},
      {"Uncensored",           {Other}},
      {"Uncut",                {Other}},
      {"TS",                   {Other}},
      {"VFR",                  {Other}},
      {"Widescreen",           {Other}},
      {"WS",                   {Other}},

      // Release group
      {"THORA",                {ReleaseGroup}},  // special case because usually placed at the end

      // Release information
      {"Batch",                {ReleaseInformation}},
      {"Complete",             {ReleaseInformation}},
      {"End",                  {ReleaseInformation, Ambiguous}},  // e.g. "The End of Evangelion"
      {"Final",                {ReleaseInformation, Ambiguous}},  // e.g. "Final Approach"
      {"Patch",                {ReleaseInformation}},
      {"Remux",                {ReleaseInformation}},

      // Release version
      {"v0",                   {ReleaseVersion}},
      {"v1",                   {ReleaseVersion}},
      {"v2",                   {ReleaseVersion}},
      {"v3",                   {ReleaseVersion}},
      {"v4",                   {ReleaseVersion}},

      // Season
      // Usually preceded or followed by a number (e.g. `2nd Season` or `Season 2`).
      {"Season",               {Season, Ambiguous}},
      {"Saison",               {Season, Ambiguous}},

      // Source
      //
      // Blu-ray
      {"BD",                   {Source}},
      {"BDRip",                {Source}},
      {"BluRay",               {Source}},
      {"Blu-ray",              {Source}},
      // DVD
      {"DVD",                  {Source}},
      {"DVD5",                 {Source}},
      {"DVD9",                 {Source}},
      {"DVDISO",               {Source}},
      {"DVDRip",               {Source}},
      {"DVD-Rip",              {Source}},
      {"R2DVD",                {Source}},
      {"R2J",                  {Source}},
      {"R2JDVD",               {Source}},
      {"R2JDVDRip",            {Source}},
      // TV
      {"HDTV",                 {Source}},
      {"HDTVRip",              {Source}},
      {"TVRip",                {Source}},
      {"TV-Rip",               {Source}},
      // Web
      {"Web",                  {Source, Ambiguous}},
      {"Webcast",              {Source}},
      {"WebDL",                {Source}},
      {"Web-DL",               {Source}},
      {"WebRip",               {Source}},
      {"ADN",                  {Source}},  // Animation Digital Network
      {"AMZN",                 {Source}},  // Amazon Prime
      {"CR",                   {Source}},  // Crunchyroll
      {"Crunchyroll",          {Source}},
      {"DSNP",                 {Source}},  // Disney+
      {"Funi",                 {Source}},  // Funimation
      {"Funimation",           {Source}},
      {"HIDI",                 {Source}},  // Hidive
      {"Hidive",               {Source}},
      {"Hulu",                 {Source}},
      {"Netflix",              {Source}},
      {"NF",                   {Source}},  // Netflix
      {"VRV",                  {Source}},
      {"YouTube",              {Source}},

      // Subtitles
      {"ASS",                  {Subtitles}},
      {"BIG5",                 {Subtitles}},
      {"Dub",                  {Subtitles}},
      {"Dubbed",               {Subtitles}},
      {"Hardsub",              {Subtitles}},
      {"Hardsubs",             {Subtitles}},
      {"RAW",                  {Subtitles}},
      {"Softsub",              {Subtitles}},
      {"Softsubs",             {Subtitles}},
      {"Sub",                  {Subtitles}},
      {"Subbed",               {Subtitles}},
      {"Subtitled",            {Subtitles}},
      {"Multisub",             {Subtitles}},
      {"Multi Sub",            {Subtitles}},
      {"Multi Subs",           {Subtitles}},
      {"Multi-Subs",           {Subtitles}},
      {"Multiple Subtitle",    {Subtitles}},
      {"EngSub",               {Subtitles}},
      {"EngSubs",              {Subtitles}},
      {"GerSub",               {Subtitles}},

      // Type
      {"TV",                   {Type, Ambiguous}},
      {"Movie",                {Type, Ambiguous}},
      {"Gekijouban",           {Type, Ambiguous}},
      {"OAD",                  {Type, Ambiguous}},
      {"OAV",                  {Type, Ambiguous}},
      {"ONA",                  {Type, Ambiguous}},
      {"OVA",                  {Type, Ambiguous}},
      {"SP",                   {Type, Ambiguous}},  // e.g. "Yumeiro Patissiere SP Professional"
      {"Special",              {Type, Ambiguous}},  // e.g. "Special A"
      {"Specials",             {Type, Ambiguous}},

      // Video
      //
      // Color depth
      {"8bit",                 {VideoColorDepth}},
      {"8-bit",                {VideoColorDepth}},
      {"10bit",                {VideoColorDepth}},
      {"10bits",               {VideoColorDepth}},
      {"10-bit",               {VideoColorDepth}},
      {"10-bits",              {VideoColorDepth}},
      // Codec
      {"AV1",                  {VideoCodec}},
      {"AVC",                  {VideoCodec}},
      {"DivX",                 {VideoCodec}},  // @Warning: Duplicate
      {"DivX5",                {VideoCodec}},
      {"DivX6",                {VideoCodec}},
      {"H.264",                {VideoCodec}},
      {"H.265",                {VideoCodec}},
      {"X.264",                {VideoCodec}},
      {"H264",                 {VideoCodec}},
      {"H265",                 {VideoCodec}},
      {"X264",                 {VideoCodec}},
      {"X265",                 {VideoCodec}},
      {"HEVC",                 {VideoCodec}},
      {"HEVC2",                {VideoCodec}},
      {"Xvid",                 {VideoCodec}},
      {"HDR",                  {VideoCodec}},
      {"DV",                   {VideoCodec}},
      {"Dolby Vision",         {VideoCodec}},
      // Format
      {"AVI",                  {VideoFormat}},  // @Warning: Duplicate
      {"RMVB",                 {VideoFormat}},  // @Warning: Duplicate
      {"WMV",                  {VideoFormat}},  // @Warning: Duplicate
      {"WMV3",                 {VideoFormat}},
      {"WMV9",                 {VideoFormat}},
      // Frame rate
      {"23.976FPS",            {VideoFrameRate}},
      {"24FPS",                {VideoFrameRate}},
      {"29.97FPS",             {VideoFrameRate}},
      {"30FPS",                {VideoFrameRate}},
      {"60FPS",                {VideoFrameRate}},
      {"120FPS",               {VideoFrameRate}},
      // Profile
      {"Hi10",                 {VideoProfile}},
      {"Hi10p",                {VideoProfile}},
      {"Hi444",                {VideoProfile}},
      {"Hi444P",               {VideoProfile}},
      {"Hi444PP",              {VideoProfile}},
      // Quality
      {"HD",                   {VideoQuality}},
      {"SD",                   {VideoQuality}},
      {"HQ",                   {VideoQuality}},
      {"LQ",                   {VideoQuality}},
      // Resolution
      {"1080p",                {VideoResolution, Unbounded}},
      {"1440p",                {VideoResolution, Unbounded}},
      {"2160p",                {VideoResolution, Unbounded}},
      {"4K",                   {VideoResolution}},

      // Volume
      {"Vol",                  {Volume}},
      {"Volume",               {Volume}},
  };
  // clang-format on
}();

}  // namespace anitomy::detail
