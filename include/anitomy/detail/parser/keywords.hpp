#pragma once

#include <ranges>
#include <span>
#include <string>
#include <vector>

#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>
#include <anitomy/options.hpp>

namespace anitomy::detail {

inline std::vector<Element> parse_keywords(std::span<Token> tokens,
                                           const Options& options) noexcept {
  static constexpr auto filter = std::views::filter;

  static constexpr auto to_element_kind = [](const KeywordKind kind) {
    using K = KeywordKind;
    using E = ElementKind;
    // clang-format off
    switch (kind) {
      case K::AudioChannels:      return E::AudioTerm;
      case K::AudioCodec:         return E::AudioTerm;
      case K::AudioLanguage:      return E::AudioTerm;
      case K::Device:             return E::Device;
      case K::Episode:            return E::Episode;
      case K::EpisodeType:        return E::Type;
      case K::Language:           return E::Language;
      case K::Other:              return E::Other;
      case K::ReleaseGroup:       return E::ReleaseGroup;
      case K::ReleaseInformation: return E::ReleaseInformation;
      case K::ReleaseVersion:     return E::ReleaseVersion;
      case K::Season:             return E::Season;
      case K::Source:             return E::Source;
      case K::Subtitles:          return E::Subtitles;
      case K::Type:               return E::Type;
      case K::VideoCodec:         return E::VideoTerm;
      case K::VideoColorDepth:    return E::VideoTerm;
      case K::VideoDynamicRange:  return E::VideoTerm;
      case K::VideoFormat:        return E::VideoTerm;
      case K::VideoFrameRate:     return E::VideoTerm;
      case K::VideoProfile:       return E::VideoTerm;
      case K::VideoQuality:       return E::VideoTerm;
      case K::VideoResolution:    return E::VideoResolution;
      case K::Volume:             return E::Volume;
    }
    // clang-format on
    return E::Other;
  };

  static constexpr auto is_prefix = [](const KeywordKind kind) {
    using enum KeywordKind;
    return kind == Episode || kind == Season || kind == Volume;
  };

  const auto is_allowed = [&options](const Token& token) {
    if (!token.keyword) return false;
    if (token.keyword->kind == KeywordKind::ReleaseGroup) return options.parse_release_group;
    if (token.keyword->kind == KeywordKind::VideoResolution) return options.parse_video_resolution;
    return true;
  };

  static constexpr auto token_value = [](const Token& token) -> std::string {
    if (token.keyword->kind == KeywordKind::ReleaseVersion) {
      return token.value.substr(1);  // `v2` -> `2`
    }
    return token.value;
  };

  std::vector<Element> elements;

  for (auto& token : tokens | filter(is_keyword_token) | filter(is_allowed)) {
    const auto element_kind = to_element_kind(token.keyword->kind);
    if (!token.keyword->is_ambiguous() || token.is_enclosed) {
      token.element_kind = element_kind;
    }
    if (!is_prefix(token.keyword->kind)) {
      elements.emplace_back(element_kind, token_value(token), token.position);
    }
  }

  return elements;
}

}  // namespace anitomy::detail
