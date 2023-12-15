#pragma once

#include <map>
#include <ranges>
#include <regex>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "container.hpp"
#include "delimiter.hpp"
#include "element.hpp"
#include "options.hpp"
#include "parser/anime_season.hpp"
#include "parser/anime_year.hpp"
#include "parser/file_checksum.hpp"
#include "parser/file_extension.hpp"
#include "parser/video_resolution.hpp"
#include "parser/volume_number.hpp"
#include "token.hpp"
#include "util.hpp"

namespace anitomy::detail {

class Parser final : public ElementContainer, public TokenContainer {
public:
  static constexpr auto filter = std::views::filter;
  static constexpr auto reverse = std::views::reverse;
  static constexpr auto take = std::views::take;

  explicit Parser(std::vector<Token>& tokens) : TokenContainer{tokens} {
  }

  inline void parse(const Options& options) noexcept {
    if (options.parse_file_extension) {
      if (auto element = parse_file_extension(tokens_)) {
        elements_.emplace_back(*element);
      }
    }

    search_keywords(options);

    if (options.parse_file_checksum) {
      if (auto element = parse_file_checksum(tokens_)) {
        elements_.emplace_back(*element);
      }
    }

    if (options.parse_video_resolution) {
      if (auto elements = parse_video_resolution(tokens_); !elements.empty()) {
        std::ranges::move(elements, std::back_inserter(elements_));
      }
    }

    if (options.parse_anime_year) {
      if (auto element = parse_anime_year(tokens_)) {
        elements_.emplace_back(*element);
      }
    }

    if (options.parse_anime_season) {
      if (auto element = parse_anime_season(tokens_)) {
        elements_.emplace_back(*element);
      }
    }

    if (options.parse_episode_number) {
      if (auto element = parse_volume_number(tokens_)) {
        elements_.emplace_back(*element);
      }

      search_episode_number();
    }

    if (options.parse_anime_title) {
      search_anime_title();
    }

    if (options.parse_release_group && !contains(ElementKind::ReleaseGroup)) {
      search_release_group();
    }

    if (options.parse_episode_title && contains(ElementKind::EpisodeNumber)) {
      search_episode_title();
    }
  }

private:
  inline void search_keywords(const Options& options) noexcept {
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

    for (auto& token : tokens_ | filter(is_keyword_token)) {
      if (!is_allowed(token)) continue;
      if (const auto it = table.find(token.keyword->kind); it != table.end()) {
        const bool identify_token = token.keyword->is_identifiable();
        add_element_from_token(it->second, token, token_value(token), identify_token);
      }
    }
  }

  inline void search_episode_number() noexcept {
    // episode prefix (e.g. `E1`, `EP1`, `Episode 1`...)
    {
      static constexpr auto is_episode_keyword = [](const Token& token) {
        return token.keyword && token.keyword->kind == KeywordKind::Episode;
      };

      auto episode_token = std::ranges::find_if(tokens_, is_episode_keyword);

      // Check next token for a number
      if (auto token = find_next_token(episode_token, is_not_delimiter_token);
          token != tokens_.end()) {
        if (is_free_token(*token) && is_numeric_token(*token)) {
          add_element_from_token(ElementKind::EpisodeNumber, *token);
          episode_token->element_kind = ElementKind::EpisodeNumber;
          return;
        }
      }
    }
    {
      static constexpr auto is_episode_prefix = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((?:E|E[Pp]|Eps)(\d{1,4})(?:[vV](\d))?)"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_episode_prefix(token, matches)) {
          add_element_from_token(ElementKind::EpisodeNumber, token, matches[1].str());
          if (matches[2].matched) add_element(ElementKind::ReleaseVersion, matches[2].str());
          return;
        }
      }
    }

    // number comes before another number (e.g. `8 & 10`, `01 of 24`)
    {
      auto tokens = tokens_ | filter(is_free_token) | filter(is_numeric_token);

      for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        // skip if delimiter but not '&'
        auto token = std::ranges::find_if(
            std::next(it.base().base()), tokens_.end(),
            [](const Token& token) { return is_not_delimiter_token(token) || token.value == "&"; });
        if (token == tokens_.end()) continue;
        // check if '&' or "of"
        if (token->value != "&" && token->value != "of") continue;
        // skip if delimiter
        auto next_token = find_next_token(token, is_not_delimiter_token);
        if (next_token == tokens_.end()) continue;
        // check if number
        if (!is_numeric_token(*next_token)) continue;
        add_element_from_token(ElementKind::EpisodeNumber, *it);
        add_element_from_token(ElementKind::EpisodeNumber, *next_token);
        return;
      }
    }

    // single episode (e.g. `01v2`)
    {
      static constexpr auto is_single_episode = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((\d{1,4})[vV](\d))"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_single_episode(token, matches)) {
          add_element_from_token(ElementKind::EpisodeNumber, token, matches[1].str());
          add_element(ElementKind::ReleaseVersion, matches[2].str());
          return;
        }
      }
    }

    // multi episode (e.g. `01-02`, `03-05v2`)
    {
      static constexpr auto is_multi_episode = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{R"((\d{1,4})(?:[vV](\d))?[-~&+])"
                                        R"((\d{1,4})(?:[vV](\d))?)"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_multi_episode(token, matches)) {
          auto lower = matches[1].str();
          auto upper = matches[3].str();
          if (to_int(lower) >= to_int(upper)) continue;  // avoid matching `009-1`, `5-2`, etc.
          add_element_from_token(ElementKind::EpisodeNumber, token, lower);
          if (matches[2].matched) add_element(ElementKind::ReleaseVersion, matches[2].str());
          add_element_from_token(ElementKind::EpisodeNumber, token, upper);
          if (matches[4].matched) add_element(ElementKind::ReleaseVersion, matches[4].str());
          return;
        }
      }
    }

    // season and episode (e.g. `2x01`, `S01E03`, `S01-02xE001-150`)
    {
      static constexpr auto is_season_and_episode = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{
            "S?"
            "(\\d{1,2})(?:-S?(\\d{1,2}))?"
            "(?:x|[ ._-x]?E)"
            "(\\d{1,4})(?:-E?(\\d{1,4}))?"
            "(?:[vV](\\d))?"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_season_and_episode(token, matches)) {
          if (to_int(matches[1].str()) == 0) continue;
          add_element(ElementKind::AnimeSeason, matches[1].str());
          if (matches[2].matched) add_element(ElementKind::AnimeSeason, matches[2].str());
          add_element_from_token(ElementKind::EpisodeNumber, token, matches[3].str());
          if (matches[4].matched) add_element(ElementKind::EpisodeNumber, matches[4].str());
          if (matches[5].matched) add_element(ElementKind::ReleaseVersion, matches[5].str());
          return;
        }
      }
    }

    // type and episode (e.g. `ED1`, `OP4a`, `OVA2`)
    {
      static constexpr auto is_type_keyword = [](const Token& token) {
        return token.keyword && token.keyword->kind == KeywordKind::AnimeType;
      };

      auto type_token = std::ranges::find_if(tokens_, is_type_keyword);

      // Check next token for a number
      if (auto token = find_next_token(type_token, is_not_delimiter_token);
          token != tokens_.end()) {
        if (is_free_token(*token) && is_numeric_token(*token)) {
          add_element_from_token(ElementKind::EpisodeNumber, *token);
          return;
        }
      }
    }

    // fractional episode (e.g. `07.5`)
    {
      for (auto [number, delimiter, fraction] : tokens_ | std::views::adjacent<3>) {
        if (is_free_token(number) && is_numeric_token(number)) {
          if (is_delimiter_token(delimiter) && delimiter.value == ".") {
            // We don't allow any fractional part other than `.5`, because there are cases
            // where such a number is a part of the anime title (e.g. `Evangelion: 1.11`,
            // `Tokyo Magnitude 8.0`) or a keyword (e.g. `5.1`).
            if (is_free_token(fraction) && fraction.value == "5") {
              add_element_from_token(
                  ElementKind::EpisodeNumber, number,
                  std::format("{}{}{}", number.value, delimiter.value, fraction.value));
              delimiter.element_kind = ElementKind::EpisodeNumber;
              fraction.element_kind = ElementKind::EpisodeNumber;
              return;
            }
          }
        }
      }
    }

    // number sign (e.g. `#01`, `#02-03v2`)
    {
      static constexpr auto is_number_sign = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{"#(\\d{1,4})(?:[-~&+](\\d{1,4}))?(?:[vV](\\d))?"};
        return token.value.starts_with('#') && std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_number_sign(token, matches)) {
          add_element_from_token(ElementKind::EpisodeNumber, token, matches[1].str());
          if (matches[2].matched) add_element(ElementKind::EpisodeNumber, matches[2].str());
          if (matches[3].matched) add_element(ElementKind::ReleaseVersion, matches[3].str());
          return;
        }
      }
    }

    // japanese counter (e.g. `第01話`)
    {
      static constexpr auto is_japanese_counter = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{"(?:第)?(\\d{1,4})話"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_japanese_counter(token, matches)) {
          add_element_from_token(ElementKind::EpisodeNumber, token, matches[1].str());
          return;
        }
      }
    }

    // equivalent numbers (e.g. `01 (176)`, `29 (04)`)
    {
      // @TODO
    }

    // separated number (e.g. ` - 08`)
    {
      static constexpr auto is_dash_token = [](const Token& token) {
        return token.kind == TokenKind::Delimiter && is_dash(token.value.front());
      };

      auto tokens = tokens_ | filter(is_dash_token);

      for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        auto next_token = std::ranges::find_if(it.base(), tokens_.end(), is_not_delimiter_token);
        if (next_token != tokens_.end() && is_numeric_token(*next_token)) {
          add_element_from_token(ElementKind::EpisodeNumber, *next_token);
          return;
        }
      }
    }

    // isolated number (e.g. `[12]`, `(2006)`)
    {
      using window_t = std::tuple<Token&, Token&, Token&>;

      static constexpr auto is_isolated = [](window_t tokens) {
        return std::get<0>(tokens).kind == TokenKind::OpenBracket &&
               std::get<2>(tokens).kind == TokenKind::CloseBracket;
      };

      static constexpr auto is_free_number = [](window_t tokens) {
        auto& token = std::get<1>(tokens);
        return is_free_token(token) && is_numeric_token(token);
      };

      auto tokens = tokens_ | std::views::adjacent<3> | filter(is_isolated) |
                    filter(is_free_number) | take(1);

      if (!tokens.empty()) {
        add_element_from_token(ElementKind::EpisodeNumber, std::get<1>(tokens.front()));
        return;
      }
    }

    // partial episode (e.g. `4a`, `111C`)
    {
      static constexpr auto is_partial_episode = [](const Token& token) {
        static const std::regex pattern{R"(\d{1,4}[ABCabc])"};
        return std::regex_match(token.value, pattern);
      };

      auto tokens = tokens_ | filter(is_free_token) | filter(is_partial_episode) | take(1);
      if (!tokens.empty()) {
        add_element_from_token(ElementKind::EpisodeNumber, tokens.front());
        return;
      }
    }

    // last number
    // @TODO: should not parse `1.11`, `Part 2`
    {
      auto tokens = tokens_ | reverse | filter(is_free_token) | filter(is_numeric_token) | take(1);

      if (!tokens.empty()) {
        add_element_from_token(ElementKind::EpisodeNumber, tokens.front());
        return;
      }
    }
  }

  inline void search_anime_title() noexcept {
    // Find the first free unenclosed token
    auto token_begin = std::ranges::find_if(
        tokens_, [](const Token& token) { return is_free_token(token) && !token.is_enclosed; });

    // If that doesn't work, find the first unknown token in the second enclosed group (assuming
    // that the first one is the release group)
    if (token_begin == tokens_.end()) {
      // @TODO
      return;
    }

    // Continue until an identifier is found
    auto token_end = std::ranges::find_if(token_begin, tokens_.end(), [](const Token& token) {
      return token.element_kind.has_value();
    });

    // If the interval ends with an enclosed group (e.g. "Anime Title [Fansub]"), move the upper
    // endpoint back to the beginning of the group. We ignore parentheses in order to keep certain
    // groups (e.g. "(TV)") intact.
    //
    // @TODO

    // Trim delimiters and open brackets
    static constexpr auto is_invalid_token = [](const Token& token) {
      return token.kind == TokenKind::Delimiter || token.kind == TokenKind::OpenBracket;
    };
    while (token_end != tokens_.begin() && token_end != tokens_.end() &&
           is_invalid_token(*std::prev(token_end))) {
      token_end = std::prev(token_end);
    }

    auto span = std::span(token_begin, token_end);

    // Build anime title
    if (std::string value = build_element_value(span); !value.empty()) {
      add_element_from_tokens(ElementKind::AnimeTitle, span, value);
    }
  }

  constexpr void search_release_group() noexcept {
    auto token_begin = tokens_.begin();
    auto token_end = tokens_.begin();

    do {
      // Find the first free enclosed token
      token_begin = std::ranges::find_if(token_end, tokens_.end(), [](const Token& token) {
        return is_free_token(token) && token.is_enclosed;
      });
      if (token_begin == tokens_.end()) return;

      // Continue until a bracket or identifier is found
      token_end = std::ranges::find_if(token_begin, tokens_.end(), [](const Token& token) {
        return token.kind == TokenKind::CloseBracket || token.element_kind.has_value();
      });
      if (token_end == tokens_.end()) return;
      if (token_end->kind != TokenKind::CloseBracket) continue;

      // @TODO: Ignore if it's not the first non-delimiter token in group

      auto span = std::span(token_begin, token_end);

      // Build release group
      if (std::string value = build_element_value(span, false); !value.empty()) {
        add_element_from_tokens(ElementKind::ReleaseGroup, span, value);
      }
      return;
    } while (token_begin != tokens_.end());
  }

  // @TODO: https://github.com/erengy/anitomy/issues/13
  constexpr void search_episode_title() noexcept {
    auto token_begin = tokens_.begin();
    auto token_end = tokens_.begin();

    do {
      // Find the first free unenclosed token
      token_begin = std::ranges::find_if(token_end, tokens_.end(), [](const Token& token) {
        return is_free_token(token) && !token.is_enclosed;
      });
      if (token_begin == tokens_.end()) return;

      // Continue until a bracket or identifier is found
      token_end = std::ranges::find_if(token_begin, tokens_.end(), [](const Token& token) {
        return token.kind == TokenKind::OpenBracket || token.element_kind.has_value();
      });

      // @TODO: Ignore if it's only a dash

      auto span = std::span(token_begin, token_end);

      // Trim delimiters
      while (span.back().kind == TokenKind::Delimiter) {
        span = span.first(span.size() - 1);
      }

      // Build episode title
      if (std::string value = build_element_value(span); !value.empty()) {
        add_element_from_tokens(ElementKind::EpisodeTitle, span, value);
      }
      return;
    } while (token_begin != tokens_.end());
  }
};

}  // namespace anitomy::detail
