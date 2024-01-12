#pragma once

#include <map>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>
#include <anitomy/options.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_keywords(std::span<Token> tokens,
                                           const Options& options) noexcept {
  static constexpr auto filter = std::views::filter;

  static const std::map<KeywordKind, ElementKind> table{
      // clang-format off
      {KeywordKind::AudioChannels,       ElementKind::AudioTerm},
      {KeywordKind::AudioCodec,          ElementKind::AudioTerm},
      {KeywordKind::AudioLanguage,       ElementKind::AudioTerm},
      {KeywordKind::DeviceCompatibility, ElementKind::DeviceCompatibility},
      {KeywordKind::EpisodeType,         ElementKind::Type},
      {KeywordKind::Language,            ElementKind::Language},
      {KeywordKind::Other,               ElementKind::Other},
      {KeywordKind::ReleaseGroup,        ElementKind::ReleaseGroup},
      {KeywordKind::ReleaseInformation,  ElementKind::ReleaseInformation},
      {KeywordKind::ReleaseVersion,      ElementKind::ReleaseVersion},
      {KeywordKind::Source,              ElementKind::Source},
      {KeywordKind::Subtitles,           ElementKind::Subtitles},
      {KeywordKind::Type,                ElementKind::Type},
      {KeywordKind::VideoCodec,          ElementKind::VideoTerm},
      {KeywordKind::VideoColorDepth,     ElementKind::VideoTerm},
      {KeywordKind::VideoFormat,         ElementKind::VideoTerm},
      {KeywordKind::VideoFrameRate,      ElementKind::VideoTerm},
      {KeywordKind::VideoProfile,        ElementKind::VideoTerm},
      {KeywordKind::VideoQuality,        ElementKind::VideoTerm},
      {KeywordKind::VideoResolution,     ElementKind::VideoResolution},
      // clang-format on
  };

  static const auto is_allowed = [&options](const Token& token) {
    if (!token.keyword) {
      return false;
    }
    switch (token.keyword->kind) {
      case KeywordKind::ReleaseGroup:
        return options.parse_release_group;
      case KeywordKind::VideoResolution:
        return options.parse_video_resolution;
    }
    return true;
  };

  static constexpr auto token_value = [](const Token& token) -> std::string {
    switch (token.keyword->kind) {
      case KeywordKind::ReleaseVersion:
        return token.value.substr(1);  // `v2` -> `2`
    }
    return token.value;
  };

  std::vector<Element> elements;

  for (auto& token : tokens | filter(is_keyword_token) | filter(is_allowed)) {
    if (const auto it = table.find(token.keyword->kind); it != table.end()) {
      if (!token.keyword->is_ambiguous() || token.is_enclosed) {
        token.element_kind = it->second;
      }
      elements.emplace_back(it->second, token_value(token), token.position);
    }
  }

  return elements;
}

}  // namespace anitomy::detail
