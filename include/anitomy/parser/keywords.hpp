#pragma once

#include <map>
#include <ranges>
#include <vector>

#include "../element.hpp"
#include "../options.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::vector<Element> parse_keywords(std::vector<Token>& tokens_,
                                           const Options& options) noexcept {
  std::vector<Element> elements;

  static const std::map<KeywordKind, ElementKind> table{
      {KeywordKind::AnimeType, ElementKind::AnimeType},
      {KeywordKind::AudioChannels, ElementKind::AudioTerm},
      {KeywordKind::AudioCodec, ElementKind::AudioTerm},
      {KeywordKind::AudioLanguage, ElementKind::AudioTerm},
      {KeywordKind::DeviceCompatibility, ElementKind::DeviceCompatibility},
      {KeywordKind::EpisodeType, ElementKind::AnimeType},
      {KeywordKind::Language, ElementKind::Language},
      {KeywordKind::Other, ElementKind::Other},
      {KeywordKind::ReleaseGroup, ElementKind::ReleaseGroup},
      {KeywordKind::ReleaseInformation, ElementKind::ReleaseInformation},
      {KeywordKind::ReleaseVersion, ElementKind::ReleaseVersion},
      {KeywordKind::Source, ElementKind::Source},
      {KeywordKind::Subtitles, ElementKind::Subtitles},
      {KeywordKind::VideoCodec, ElementKind::VideoTerm},
      {KeywordKind::VideoColorDepth, ElementKind::VideoTerm},
      {KeywordKind::VideoFormat, ElementKind::VideoTerm},
      {KeywordKind::VideoFrameRate, ElementKind::VideoTerm},
      {KeywordKind::VideoProfile, ElementKind::VideoTerm},
      {KeywordKind::VideoQuality, ElementKind::VideoTerm},
      {KeywordKind::VideoResolution, ElementKind::VideoResolution},
  };

  static const auto is_allowed = [&options](const Token& token) {
    if (!token.keyword) return false;
    switch (token.keyword->kind) {
      case KeywordKind::ReleaseGroup:
        return options.parse_release_group;
      case KeywordKind::VideoResolution:
        return options.parse_video_resolution;
    }
    return true;
  };

  static constexpr auto token_value = [](const Token& token) -> std::string_view {
    if (token.keyword->kind == KeywordKind::ReleaseVersion) {
      std::string_view view{token.value};
      if (view.starts_with('v') || view.starts_with('V')) view.remove_prefix(1);
      return view;
    }
    return token.value;
  };

  for (auto& token : tokens_ | std::views::filter(is_keyword_token)) {
    if (!is_allowed(token)) continue;
    if (const auto it = table.find(token.keyword->kind); it != table.end()) {
      if (token.keyword->is_identifiable()) token.element_kind = it->second;
      elements.emplace_back(it->second, std::string{token_value(token)});
    }
  }

  return elements;
}

}  // namespace anitomy::detail
