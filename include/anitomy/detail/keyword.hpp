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
  Device,
  Episode,
  EpisodeType,
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
  VideoDynamicRange,
  VideoFormat,
  VideoFrameRate,
  VideoProfile,
  VideoQuality,
  VideoResolution,
  Volume,
};

struct Keyword {
  enum Flags : uint8_t {
    Ambiguous = 1 << 0,
    Subword = 1 << 1,
    PrefixForNumber = 1 << 2,
  };

  KeywordKind kind;
  uint8_t flags = 0;

  constexpr bool is_ambiguous() const noexcept {
    return has_flag(Ambiguous);
  }

  constexpr bool is_subword() const noexcept {
    return has_flag(Subword);
  }

  constexpr bool is_prefix_for_number() const noexcept {
    return has_flag(PrefixForNumber);
  }

private:
  constexpr bool has_flag(const Flags flag) const noexcept {
    return (flags & flag) == flag;
  }
};

struct KeywordHash {
  [[nodiscard]] size_t operator()(const std::string& view) const noexcept {
    auto str = view | std::views::transform(to_lower<char>) | std::ranges::to<std::string>();
    return std::hash<std::string>()(str);
  }
};

struct KeywordEqual {
  [[nodiscard]] bool operator()(const std::string& a, const std::string& b) const noexcept {
    return std::ranges::equal(a, b, equal_to);
  }
};

using keyword_map_t = std::unordered_map<std::string, Keyword, KeywordHash, KeywordEqual>;

inline keyword_map_t make_keywords() noexcept {
  using enum KeywordKind;
  using enum Keyword::Flags;

  // clang-format off
  keyword_map_t keywords{
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
      {"MultiAudio",           {AudioLanguage}},
      {"Multi Audio",          {AudioLanguage}},
      {"Dub",                  {AudioLanguage}},
      {"Dubbed",               {AudioLanguage}},
      {"Dubs",                 {AudioLanguage}},
      {"ChiDub",               {AudioLanguage}},
      {"EngDub",               {AudioLanguage}},
      {"GerDub",               {AudioLanguage}},
      {"JapDub",               {AudioLanguage}},
      {"Chinese Dub",          {AudioLanguage}},
      {"English Dub",          {AudioLanguage}},
      {"German Dub",           {AudioLanguage}},
      {"Japanese Dub",         {AudioLanguage}},
      {"Korean Dub",           {AudioLanguage}},

      // Device
      {"Android",              {Device, Ambiguous}},  // e.g. "Dragon Ball Z: Super Android 13"
      {"iPad3",                {Device}},
      {"iPhone5",              {Device}},
      {"iPod",                 {Device}},
      {"PS3",                  {Device}},
      {"Xbox",                 {Device}},
      {"Xbox360",              {Device}},

      // Episode prefix
      {"Ep",                   {Episode, PrefixForNumber}},
      {"Eps",                  {Episode, PrefixForNumber}},
      {"Episode",              {Episode, PrefixForNumber}},
      {"Episodes",             {Episode, PrefixForNumber}},
      {"Episodio",             {Episode, PrefixForNumber}},
      {"Episódio",             {Episode, PrefixForNumber}},
      {"Capitulo",             {Episode, PrefixForNumber}},
      {"Folge",                {Episode, PrefixForNumber}},

      // Episode type
      {"OP",                   {EpisodeType, Ambiguous | PrefixForNumber}},  // e.g. "takt op.Destiny", "My Unique Skill Makes Me OP even at Level 1"
      {"Opening",              {EpisodeType, Ambiguous}},                    // e.g. "Pool Opening"
      {"ED",                   {EpisodeType, Ambiguous | PrefixForNumber}},  // e.g. "s.CRY.ed"
      {"Ending",               {EpisodeType, Ambiguous}},                    // e.g. "Happy Ending", "True Ending"
      {"NCED",                 {EpisodeType, PrefixForNumber}},
      {"NCOP",                 {EpisodeType, PrefixForNumber}},
      {"Preview",              {EpisodeType, Ambiguous}},
      {"PV",                   {EpisodeType, Ambiguous | PrefixForNumber}},

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
      {"Blu ray",              {Source}},
      // DVD
      {"DVD",                  {Source}},
      {"DVD5",                 {Source}},
      {"DVD9",                 {Source}},
      {"DVDISO",               {Source}},
      {"DVDRip",               {Source}},
      {"DVD Rip",              {Source}},
      {"R2DVD",                {Source}},
      {"R2J",                  {Source}},
      {"R2JDVD",               {Source}},
      {"R2JDVDRip",            {Source}},
      // TV
      {"HDTV",                 {Source}},
      {"HDTVRip",              {Source}},
      {"TVRip",                {Source}},
      {"TV Rip",               {Source}},
      // Web
      {"Web",                  {Source, Ambiguous}},
      {"Webcast",              {Source}},
      {"WebDL",                {Source}},
      {"Web DL",               {Source}},
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
      {"Multiple Subtitle",    {Subtitles}},
      {"EngSub",               {Subtitles}},
      {"EngSubs",              {Subtitles}},
      {"GerSub",               {Subtitles}},

      // Type
      {"TV",                   {Type, Ambiguous}},
      {"Movie",                {Type, Ambiguous}},
      {"Gekijouban",           {Type, Ambiguous}},
      {"OAD",                  {Type, Ambiguous | PrefixForNumber}},
      {"OAV",                  {Type, Ambiguous | PrefixForNumber}},
      {"ONA",                  {Type, Ambiguous | PrefixForNumber}},
      {"OVA",                  {Type, Ambiguous | PrefixForNumber}},
      {"SP",                   {Type, Ambiguous | PrefixForNumber}},  // e.g. "Yumeiro Patissiere SP Professional"
      {"Special",              {Type, Ambiguous}},                    // e.g. "Special A"
      {"Specials",             {Type, Ambiguous}},

      // Video
      //
      // Color depth
      {"8bit",                 {VideoColorDepth}},
      {"8bits",                {VideoColorDepth}},
      {"8 bit",                {VideoColorDepth}},
      {"8 bits",               {VideoColorDepth}},
      {"10bit",                {VideoColorDepth}},
      {"10bits",               {VideoColorDepth}},
      {"10 bit",               {VideoColorDepth}},
      {"10 bits",              {VideoColorDepth}},
      // Codec
      {"AV1",                  {VideoCodec}},
      {"AVC",                  {VideoCodec}},
      {"DivX",                 {VideoCodec}},
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
      // Dynamic range
      {"HDR",                  {VideoDynamicRange}},
      {"HDR10",                {VideoDynamicRange}},
      {"DV",                   {VideoDynamicRange}},
      {"Dolby Vision",         {VideoDynamicRange}},
      // Format
      {"AVI",                  {VideoFormat}},
      {"RMVB",                 {VideoFormat}},
      {"WMV",                  {VideoFormat}},
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
      {"1080p",                {VideoResolution, Subword}},
      {"1440p",                {VideoResolution, Subword}},
      {"2160p",                {VideoResolution, Subword}},
      {"4K",                   {VideoResolution}},

      // Volume prefix
      {"Vol",                  {Volume, PrefixForNumber}},
      {"Volume",               {Volume, PrefixForNumber}},
  };
  // clang-format on

  constexpr auto variant = [](std::string key, const char delimiter) {
    std::ranges::replace(key, ' ', delimiter);
    return key;
  };

  for (const auto& [key, keyword] : keywords) {
    if (key.contains(' ')) {
      keywords.emplace(variant(key, '_'), keyword);
      keywords.emplace(variant(key, '.'), keyword);
      keywords.emplace(variant(key, '-'), keyword);
    }
  }

  return keywords;
};

inline auto keywords = make_keywords();

}  // namespace anitomy::detail
