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
    PrefixForOther = 1 << 3,
  };

  KeywordKind kind;
  uint8_t flags{};

  constexpr bool is_ambiguous() const noexcept {
    return has_flag(Ambiguous);
  }

  constexpr bool is_subword() const noexcept {
    return has_flag(Subword);
  }

  constexpr bool is_prefix_for_number() const noexcept {
    return has_flag(PrefixForNumber);
  }

  constexpr bool is_prefix_for_other() const noexcept {
    return has_flag(PrefixForOther);
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

constexpr auto make_base_keywords() noexcept {
  using enum KeywordKind;
  using enum Keyword::Flags;

  struct Entry {
    std::string_view value;
    uint8_t flags{};
  };

  using keywords_t = std::vector<std::pair<KeywordKind, std::vector<Entry>>>;

  // clang-format off
  return keywords_t{
      // Audio
      {
        AudioChannels, {
          {"2.0", Ambiguous},  // e.g. "Evangelion 2.0"
          {"2.0ch"},
          {"2ch"},
          {"5.1"},
          {"5.1ch"},
          {"7.1"},
          {"7.1ch"},
        }
      },
      {
        AudioCodec, {
          {"AAC", PrefixForOther},
          {"AACx2"},
          {"AACx3"},
          {"AACx4"},
          {"AC3"},
          {"EAC3"},
          {"E-AC-3"},
          {"E-AC3"},
          {"Atmos"},
          {"Dolby Atmos"},
          {"DD", PrefixForOther},
          {"DDP", PrefixForNumber},
          {"Dolby TrueHD"},
          {"TrueHD", PrefixForNumber},
          {"DTS", PrefixForNumber},
          {"DTS-ES"},
          {"FLAC", PrefixForNumber},
          {"FLACx2"},
          {"FLACx3"},
          {"FLACx4"},
          {"Lossless"},
          {"MP3"},
          {"Opus", Ambiguous},  // e.g. "Opus.COLORs"
          {"OGG"},
          {"Vorbis"},
        }
      },
      {
        AudioLanguage, {
          {"DualAudio"},
          {"Dual Audio"},
          {"MultiAudio"},
          {"Multi Audio"},
          {"Dub"},
          {"Dubbed"},
          {"Dubs"},
          {"ChiDub"},
          {"Chinese Dub"},
          {"EngDub"},
          {"English Dub"},
          {"GerDub"},
          {"German Dub"},
          {"JapDub"},
          {"Japanese Dub"},
          {"Korean Dub"},
        }
      },

      // Device
      {
        Device, {
          {"Android", Ambiguous},  // e.g. "Dragon Ball Z: Super Android 13"
          {"iPad", PrefixForNumber},
          {"iPhone", PrefixForNumber},
          {"iPod"},
          {"PS", PrefixForNumber},
          {"Xbox", PrefixForNumber},
        }
      },

      // Episode
      {
        Episode, {
          {"Ep", PrefixForNumber},
          {"Eps", PrefixForNumber},
          {"Episode", PrefixForNumber},
          {"Episodes", PrefixForNumber},
          {"Episodio", PrefixForNumber},
          {"Episódio", PrefixForNumber},
          {"Capitulo", PrefixForNumber},
          {"Folge", PrefixForNumber},
        }
      },

      // Episode type
      {
        EpisodeType, {
          {"OP", Ambiguous | PrefixForNumber},  // e.g. "takt op.Destiny", "My Unique Skill Makes Me OP even at Level 1"
          {"Opening", Ambiguous},               // e.g. "Pool Opening"
          {"NCOP", PrefixForNumber},
          {"ED", Ambiguous | PrefixForNumber},  // e.g. "s.CRY.ed"
          {"Ending", Ambiguous},                // e.g. "Happy Ending", "True Ending"
          {"NCED", PrefixForNumber},
          {"Preview", Ambiguous},
          {"PV", Ambiguous | PrefixForNumber},
        }
      },

      // Language
      {
        Language, {
          {"CHS"},  // Chinese Simplified
          {"CHT"},  // Chinese Traditional
          {"ENG"},
          {"English"},
          {"ESP", Ambiguous},  // e.g. "Tokyo ESP"
          {"Espanol"},
          {"Spanish"},
          {"ITA", Ambiguous},  // e.g. "Bokura ga Ita"
          {"JAP"},
          {"JPN"},
          {"PT-BR"},
          {"VOSTFR"},
        }
      },

      // Other
      {
        Other, {
          {"Remaster"},
          {"Remastered"},
          {"Uncensored"},
          {"Uncut"},
          {"TS"},
          {"VFR"},
          {"Widescreen"},
          {"WS"},
        }
      },

      // Release group
      {
        ReleaseGroup, {
          {"0x539"},  // to avoid parsing as season 0 episode 539
          {"THORA"},  // usually placed at the end
        }
      },

      // Release information
      {
        ReleaseInformation, {
          {"Batch"},
          {"Complete"},
          {"End", Ambiguous},    // e.g. "The End of Evangelion"
          {"Final", Ambiguous},  // e.g. "Final Approach"
          {"Patch"},
          {"Remux"},
        }
      },

      // Release version
      {
        ReleaseVersion, {
          {"v0"},
          {"v1"},
          {"v2"},
          {"v3"},
          {"v4"},
        }
      },

      // Season
      // Usually preceded or followed by a number (e.g. `2nd Season` or `Season 2`).
      {
        Season, {
          {"Season", Ambiguous},
          {"Saison", Ambiguous},
        }
      },

      // Source
      {
        Source, {
          {"BD", PrefixForOther},
          {"BDRip"},
          {"BluRay"},
          {"Blu ray"},
          {"DVD", PrefixForNumber},
          {"DVD5"},
          {"DVD9"},
          {"DVDISO"},
          {"DVDRip"},
          {"DVD Rip"},
          {"R2DVD"},
          {"R2J"},
          {"R2JDVD"},
          {"R2JDVDRip"},
          {"HDTV"},
          {"HDTVRip"},
          {"TVRip"},
          {"TV Rip"},
          {"Web", Ambiguous},
          {"Webcast"},
          {"WebDL"},
          {"Web DL"},
          {"WebRip"},
          {"ADN"},          // Animation Digital Network
          {"AMZN"},         // Amazon Prime
          {"CR"},           // Crunchyroll
          {"Crunchyroll"},
          {"DSNP"},         // Disney+
          {"Funi"},         // Funimation
          {"Funimation"},
          {"HIDI"},         // Hidive
          {"Hidive"},
          {"Hulu"},
          {"Netflix"},
          {"NF"},           // Netflix
          {"VRV"},
          {"YouTube"},
        }
      },

      // Subtitles
      {
        Subtitles, {
          {"ASS"},
          {"BIG5"},
          {"Hardsub"},
          {"Hardsubs"},
          {"RAW"},
          {"Softsub"},
          {"Softsubs"},
          {"Sub"},
          {"Subbed"},
          {"Subtitled"},
          {"Multisub"},
          {"Multi Sub"},
          {"Multi Subs"},
          {"Multiple Subtitle"},
          {"EngSub"},
          {"EngSubs"},
          {"GerSub"},
        }
      },

      // Type
      {
        Type, {
          {"TV", Ambiguous},
          {"Movie", Ambiguous},
          {"Gekijouban", Ambiguous},
          {"OAD", Ambiguous | PrefixForNumber},
          {"OAV", Ambiguous | PrefixForNumber},
          {"OVA", Ambiguous | PrefixForNumber},
          {"ONA", Ambiguous | PrefixForNumber},
          {"SP", Ambiguous | PrefixForNumber},   // e.g. "Yumeiro Patissiere SP Professional"
          {"Special", Ambiguous},                // e.g. "Special A"
          {"Specials", Ambiguous},
        }
      },

      // Video
      {
        VideoColorDepth, {
          {"8bit"},
          {"8bits"},
          {"8 bit"},
          {"8 bits"},
          {"10bit"},
          {"10bits"},
          {"10 bit"},
          {"10 bits"},
        }
      },
      {
        VideoCodec, {
          {"AV1"},
          {"DivX", PrefixForNumber},
          {"AVC"},
          {"H.264"},
          {"H264"},
          {"X.264"},
          {"X264"},
          {"H.265"},
          {"H265"},
          {"HEVC", PrefixForNumber},
          {"X265"},
          {"Xvid"},
        }
      },
      {
        VideoDynamicRange, {
          {"HDR"},
          {"HDR10"},
          {"Dolby Vision"},
          {"DV"},
        }
      },
      {
        VideoFormat, {
          {"AVI"},
          {"RMVB"},
          {"WMV"},
          {"WMV3"},
          {"WMV9"},
        }
      },
      {
        VideoFrameRate, {
          {"23.976FPS"},
          {"24FPS"},
          {"29.97FPS"},
          {"30FPS"},
          {"60FPS"},
          {"120FPS"},
        }
      },
      {
        VideoProfile, {
          {"Hi10"},
          {"Hi10p"},
          {"Hi444"},
          {"Hi444P"},
          {"Hi444PP"},
        }
      },
      {
        VideoQuality, {
          {"HD"},
          {"SD"},
          {"HQ"},
          {"LQ"},
        }
      },
      {
        VideoResolution, {
          {"1080p", Subword},
          {"1440p", Subword},
          {"2160p", Subword},
          {"4K"},
        }
      },

      // Volume
      {
        Volume, {
          {"Vol", PrefixForNumber},
          {"Volume", PrefixForNumber},
        }
      },
  };
  // clang-format on
}

inline keyword_map_t make_keywords() noexcept {
  const auto base_keywords = make_base_keywords();

  constexpr auto variant = [](std::string key, const char delimiter) {
    std::ranges::replace(key, ' ', delimiter);
    return key;
  };

  keyword_map_t keywords;

  for (const auto& [kind, entries] : base_keywords) {
    for (const auto& [value, flags] : entries) {
      Keyword keyword{kind, flags};
      keywords.emplace(std::string{value}, keyword);
      if (value.contains(' ')) {
        for (const auto delimiter : {'_', '.', '-'}) {
          keywords.emplace(variant(std::string{value}, delimiter), keyword);
        }
      }
    }
  }

  return keywords;
};

inline auto keywords = make_keywords();

}  // namespace anitomy::detail
