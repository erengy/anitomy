#pragma once

#include <string_view>
#include <unordered_map>

#include <anitomy/detail/keyword.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>

namespace anitomy::detail {

constexpr std::string_view to_string(const TokenKind kind) noexcept {
  using enum TokenKind;
  // clang-format off
  switch (kind) {
    case OpenBracket: return "open_bracket";
    case CloseBracket: return "close_bracket";
    case Delimiter: return "delimiter";
    case Keyword: return "keyword";
    case Text: return "text";
  }
  // clang-format on
  return "?";
}

constexpr std::string_view to_string(const KeywordKind kind) noexcept {
  using enum KeywordKind;
  // clang-format off
  switch (kind) {
    case AnimeSeason: return "anime_season";
    case AnimeType: return "anime_type";
    case AudioChannels: return "audio_channels";
    case AudioCodec: return "audio_codec";
    case AudioLanguage: return "audio_language";
    case DeviceCompatibility: return "device_compatibility";
    case Episode: return "episode";
    case EpisodeType: return "episode_type";
    case FileExtension: return "file_extension";
    case Language: return "language";
    case Other: return "other";
    case ReleaseGroup: return "release_group";
    case ReleaseInformation: return "release_information";
    case ReleaseVersion: return "release_version";
    case Source: return "source";
    case Subtitles: return "subtitles";
    case VideoCodec: return "video_codec";
    case VideoColorDepth: return "video_color_depth";
    case VideoFormat: return "video_format";
    case VideoFrameRate: return "video_frame_rate";
    case VideoProfile: return "video_profile";
    case VideoQuality: return "video_quality";
    case VideoResolution: return "video_resolution";
    case Volume: return "volume";
  }
  // clang-format on
  return "?";
}

constexpr std::string_view to_string(const ElementKind kind) noexcept {
  using enum ElementKind;
  // clang-format off
  switch (kind) {
    case AnimeSeason: return "anime_season";
    case AnimeTitle: return "anime_title";
    case AnimeType: return "anime_type";
    case AnimeYear: return "anime_year";
    case AudioTerm: return "audio_term";
    case DeviceCompatibility: return "device_compatibility";
    case EpisodeNumber: return "episode_number";
    case EpisodeTitle: return "episode_title";
    case FileChecksum: return "file_checksum";
    case FileExtension: return "file_extension";
    case Language: return "language";
    case Other: return "other";
    case ReleaseGroup: return "release_group";
    case ReleaseInformation: return "release_information";
    case ReleaseVersion: return "release_version";
    case Source: return "source";
    case Subtitles: return "subtitles";
    case VideoResolution: return "video_resolution";
    case VideoTerm: return "video_term";
    case VolumeNumber: return "volume_number";
  }
  // clang-format on
  return "?";
}

inline const std::optional<ElementKind> to_element_kind(std::string_view str) noexcept {
  using enum ElementKind;

  static const std::unordered_map<std::string_view, ElementKind> elements{
      {"anime_season", AnimeSeason},
      {"anime_title", AnimeTitle},
      {"anime_type", AnimeType},
      {"anime_year", AnimeYear},
      {"audio_term", AudioTerm},
      {"device_compatibility", DeviceCompatibility},
      {"episode_number", EpisodeNumber},
      {"episode_title", EpisodeTitle},
      {"file_checksum", FileChecksum},
      {"file_extension", FileExtension},
      {"language", Language},
      {"other", Other},
      {"release_group", ReleaseGroup},
      {"release_information", ReleaseInformation},
      {"release_version", ReleaseVersion},
      {"source", Source},
      {"subtitles", Subtitles},
      {"video_resolution", VideoResolution},
      {"video_term", VideoTerm},
      {"volume_number", VolumeNumber},
  };

  auto it = elements.find(str);
  return it != elements.end() ? it->second : std::optional<ElementKind>{std::nullopt};
};

}  // namespace anitomy::detail
