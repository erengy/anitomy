#pragma once

#include <ranges>
#include <regex>
#include <vector>

#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::vector<Element> parse_video_resolution(std::vector<Token>& tokens) noexcept {
  using namespace std::views;

  std::vector<Element> elements;

  // A video resolution can be in `1080p` or `1920x1080` format
  static constexpr auto is_video_resolution = [](const Token& token) {
    static const std::regex pattern{R"(\d{3,4}(?:[ip]|[xX×]\d{3,4}[ip]?))"};
    return std::regex_match(token.value, pattern);
  };

  // Find all free tokens matching the pattern
  for (auto& token : tokens | filter(is_free_token) | filter(is_video_resolution)) {
    token.element_kind = ElementKind::VideoResolution;
    elements.emplace_back(ElementKind::VideoResolution, token.value);
  }

  // If not found, look for special cases
  if (elements.empty()) {
    for (auto& token : tokens | filter(is_free_token) | filter(is_numeric_token)) {
      if (token.value == "1080") {
        token.element_kind = ElementKind::VideoResolution;
        elements.emplace_back(ElementKind::VideoResolution, token.value);
        break;
      }
    }
  }

  return elements;
}

}  // namespace anitomy::detail
