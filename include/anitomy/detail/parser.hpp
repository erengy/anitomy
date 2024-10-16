#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include <anitomy/detail/parser/episode.hpp>
#include <anitomy/detail/parser/episode_title.hpp>
#include <anitomy/detail/parser/file_checksum.hpp>
#include <anitomy/detail/parser/file_extension.hpp>
#include <anitomy/detail/parser/keywords.hpp>
#include <anitomy/detail/parser/release_group.hpp>
#include <anitomy/detail/parser/season.hpp>
#include <anitomy/detail/parser/title.hpp>
#include <anitomy/detail/parser/video_resolution.hpp>
#include <anitomy/detail/parser/volume.hpp>
#include <anitomy/detail/parser/year.hpp>
#include <anitomy/detail/token.hpp>
#include <anitomy/element.hpp>
#include <anitomy/options.hpp>

namespace anitomy::detail {

class Parser final {
public:
  explicit Parser(std::vector<Token>& tokens) : tokens_{std::move(tokens)} {
  }

  [[nodiscard]] constexpr auto&& elements(this auto&& self) noexcept {
    return std::forward<decltype(self)>(self).elements_;
  }

  [[nodiscard]] constexpr auto&& tokens(this auto&& self) noexcept {
    return std::forward<decltype(self)>(self).tokens_;
  }

  inline void parse(const Options& options) noexcept {
    // File extension
    if (options.parse_file_extension) {
      add_element(parse_file_extension(tokens_));
    }

    // Keywords
    add_elements(parse_keywords(tokens_, options));

    // Checksum
    if (options.parse_file_checksum) {
      add_element(parse_file_checksum(tokens_));
    }

    // Video resolution
    if (options.parse_video_resolution) {
      add_elements(parse_video_resolution(tokens_));
    }

    // Year
    if (options.parse_year) {
      add_element(parse_year(tokens_));
    }

    // Season
    if (options.parse_season) {
      add_element(parse_season(tokens_));
    }

    // Episode
    if (options.parse_episode) {
      add_element(parse_volume(tokens_));
      add_elements(parse_episode(tokens_));
    }

    // Title
    if (options.parse_title) {
      add_element(parse_title(tokens_));
    }

    // Release group
    if (options.parse_release_group && !contains(ElementKind::ReleaseGroup)) {
      add_element(parse_release_group(tokens_));
    }

    // Episode title
    if (options.parse_episode_title && contains(ElementKind::Episode)) {
      add_element(parse_episode_title(tokens_));
    }

    std::ranges::sort(elements_, {}, &Element::position);
  }

private:
  constexpr void add_element(std::optional<Element>&& element) noexcept {
    if (element) {
      elements_.emplace_back(*element);
    }
  }

  constexpr void add_elements(std::vector<Element>&& elements) noexcept {
    if (!elements.empty()) {
      std::ranges::move(elements, std::back_inserter(elements_));
    }
  }

  [[nodiscard]] bool contains(ElementKind kind) const noexcept {
    const auto is_kind = [&kind](const Element& element) { return element.kind == kind; };
    return std::ranges::any_of(elements_, is_kind);
  }

  std::vector<Element> elements_;
  std::vector<Token> tokens_;
};

}  // namespace anitomy::detail
