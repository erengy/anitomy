#pragma once

#include <map>
#include <ranges>
#include <regex>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "element.hpp"
#include "options.hpp"
#include "token.hpp"
#include "util.hpp"

namespace anitomy::detail {

class ElementContainer {
public:
  [[nodiscard]] const std::vector<Element>& elements() const noexcept {
    return elements_;
  }

protected:
  [[nodiscard]] static std::string build_element_value(
      const std::span<Token> tokens, const bool transform_delimiters = true) noexcept {
    std::string element_value;

    const bool has_multiple_delimiters = [&tokens]() {
      constexpr auto token_value = [](const Token& token) { return token.value.front(); };
      auto delimiters_view =
          tokens | std::views::filter(is_delimiter_token) | std::views::transform(token_value);
      std::set<char> delimiters{delimiters_view.begin(), delimiters_view.end()};
      return delimiters.size() > 1;
    }();

    const auto is_transformable = [&](const Token& token) {
      if (token.kind != TokenKind::Delimiter || !transform_delimiters) return false;
      switch (token.value.front()) {
        case ',':
        case '&':
          return false;
        case '_':
          return true;
        default:
          return !has_multiple_delimiters;
      }
    };

    for (const auto& token : tokens) {
      if (is_transformable(token)) {
        element_value.push_back(' ');
      } else {
        element_value.append(token.value);
      }
    }

    return element_value;
  }

  constexpr void add_element(ElementKind kind, std::string_view value) noexcept {
    elements_.emplace_back(kind, std::string{value});
  }

  constexpr void add_element_from_token(ElementKind kind, Token& token,
                                        std::string_view value = {}) noexcept {
    token.element_kind = kind;
    elements_.emplace_back(kind, value.empty() ? token.value : std::string{value});
  }

  constexpr void add_element_from_tokens(ElementKind kind, std::span<Token> tokens,
                                         std::string_view value) noexcept {
    for (auto& token : tokens) {
      token.element_kind = kind;
    }
    elements_.emplace_back(kind, std::string{value});
  }

private:
  std::vector<Element> elements_;
};

class Parser final : public ElementContainer, public TokenContainer {
public:
  static constexpr auto filter = std::views::filter;
  static constexpr auto reverse = std::views::reverse;
  static constexpr auto take = std::views::take;

  explicit Parser(std::vector<Token>& tokens) : TokenContainer{tokens} {
  }

  inline void parse(const Options& options) noexcept {
    if (options.parse_file_extension) search_file_extension();
    search_keywords(options);
    if (options.parse_file_checksum) search_file_checksum();
    if (options.parse_video_resolution) search_video_resolution();
    if (options.parse_anime_year) search_anime_year();
    if (options.parse_anime_season) search_anime_season();
    if (options.parse_episode_number) search_volume_number();
    if (options.parse_episode_number) search_episode_number();
    if (options.parse_anime_title) search_anime_title();
    if (options.parse_release_group) search_release_group();
    if (options.parse_episode_title) search_episode_title();
  }

private:
  inline void search_file_extension() noexcept {
    if (tokens_.size() < 2) return;

    auto tokens = tokens_ | reverse | take(2);

    if (tokens[0].keyword && tokens[0].keyword->kind == KeywordKind::FileExtension) {
      if (is_delimiter_token(tokens[1]) && tokens[1].value == ".") {
        add_element_from_token(ElementKind::FileExtension, tokens[0]);
      }
    }
  }

  inline void search_keywords(const Options& options) noexcept {
    static const std::map<KeywordKind, ElementKind> table{
        {KeywordKind::AnimeType, ElementKind::AnimeType},
        {KeywordKind::AudioTerm, ElementKind::AudioTerm},
        {KeywordKind::DeviceCompatibility, ElementKind::DeviceCompatibility},
        {KeywordKind::Language, ElementKind::Language},
        {KeywordKind::Other, ElementKind::Other},
        {KeywordKind::ReleaseGroup, ElementKind::ReleaseGroup},
        {KeywordKind::ReleaseInformation, ElementKind::ReleaseInformation},
        {KeywordKind::ReleaseVersion, ElementKind::ReleaseVersion},
        {KeywordKind::Source, ElementKind::Source},
        {KeywordKind::Subtitles, ElementKind::Subtitles},
        {KeywordKind::VideoResolution, ElementKind::VideoResolution},
        {KeywordKind::VideoTerm, ElementKind::VideoTerm},
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
        add_element_from_token(it->second, token, token_value(token));
      }
    }
  }

  inline void search_file_checksum() noexcept {
    // A checksum has 8 hexadecimal digits (e.g. `ABCD1234`)
    static constexpr auto is_checksum = [](const Token& token) {
      return token.value.size() == 8 && std::ranges::all_of(token.value, is_xdigit);
    };

    // Find the last free token that is a checksum
    auto tokens = tokens_ | reverse | filter(is_free_token) | filter(is_checksum) | take(1);
    if (!tokens.empty()) {
      add_element_from_token(ElementKind::FileChecksum, tokens.front());
    }
  }

  inline void search_video_resolution() noexcept {
    // A video resolution can be in `1080p` or `1920x1080` format
    static constexpr auto is_video_resolution = [](const Token& token) {
      static const std::regex pattern{R"(\d{3,4}p|\d{3,4}[xX×]\d{3,4})"};
      return std::regex_match(token.value, pattern);
    };

    // Find all free tokens matching the pattern
    for (auto& token : tokens_ | filter(is_free_token) | filter(is_video_resolution)) {
      add_element_from_token(ElementKind::VideoResolution, token);
    }

    // If not found, look for special cases
    if (!contains(ElementKind::VideoResolution)) {
      for (auto& token : tokens_ | filter(is_free_token) | filter(is_numeric_token)) {
        if (token.value == "1080") {
          add_element_from_token(ElementKind::VideoResolution, token);
          return;
        }
      }
    }
  }

  inline void search_anime_year() noexcept {
    using window_t = std::tuple<Token&, Token&, Token&>;

    static constexpr auto is_isolated = [](window_t tokens) {
      return std::get<0>(tokens).kind == TokenKind::OpenBracket &&
             std::get<2>(tokens).kind == TokenKind::CloseBracket;
    };

    static constexpr auto is_free_number = [](window_t tokens) {
      auto& token = std::get<1>(tokens);
      return is_free_token(token) && is_numeric_token(token);
    };

    static constexpr auto is_anime_year = [](window_t tokens) {
      const int number = to_int(std::get<1>(tokens).value);
      return 1950 < number && number < 2050;
    };

    // Find the first free isolated number within the interval
    auto tokens = tokens_ | std::views::adjacent<3> | filter(is_isolated) | filter(is_free_number) |
                  filter(is_anime_year) | take(1);
    if (!tokens.empty()) {
      add_element_from_token(ElementKind::AnimeYear, std::get<1>(tokens.front()));
    }
  }

  inline void search_anime_season() noexcept {
    using window_t = std::tuple<Token&, Token&, Token&>;

    static constexpr auto is_anime_season_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::AnimeSeason;
    };

    static constexpr auto starts_with_season_keyword = [](window_t tokens) {
      return is_anime_season_keyword(std::get<0>(tokens)) &&
             is_delimiter_token(std::get<1>(tokens)) && is_free_token(std::get<2>(tokens));
    };

    static constexpr auto ends_with_season_keyword = [](window_t tokens) {
      return is_anime_season_keyword(std::get<2>(tokens)) &&
             is_delimiter_token(std::get<1>(tokens)) && is_free_token(std::get<0>(tokens));
    };

    for (auto tokens : tokens_ | std::views::adjacent<3>) {
      // Check previous token for a number (e.g. `2nd Season`)
      if (ends_with_season_keyword(tokens)) {
        auto [token, _, season_token] = tokens;
        if (auto number = from_ordinal_number(token.value); !number.empty()) {
          add_element_from_token(ElementKind::AnimeSeason, token, number);
          season_token.element_kind = ElementKind::AnimeSeason;
          return;
        }
      }
      // Check next token for a number (e.g. `Season 2`, `Season II`)
      if (starts_with_season_keyword(tokens)) {
        auto [season_token, _, token] = tokens;
        if (is_numeric_token(token)) {
          add_element_from_token(ElementKind::AnimeSeason, token);
          season_token.element_kind = ElementKind::AnimeSeason;
          return;
        } else if (auto number = from_roman_number(token.value); !number.empty()) {
          add_element_from_token(ElementKind::AnimeSeason, token, number);
          season_token.element_kind = ElementKind::AnimeSeason;
          return;
        }
      }
    }

    // Other season patterns (e.g. `S2`, `第2期`)
    {
      static constexpr auto is_season = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{"S(\\d{1,2})"};
        return std::regex_match(token.value, matches, pattern);
      };

      static constexpr auto is_japanese_counter = [](const Token& token, std::smatch& matches) {
        static const std::regex pattern{"(?:第)?(\\d{1,2})期"};
        return std::regex_match(token.value, matches, pattern);
      };

      std::smatch matches;

      for (auto& token : tokens_ | filter(is_free_token)) {
        if (is_season(token, matches) || is_japanese_counter(token, matches)) {
          add_element_from_token(ElementKind::AnimeSeason, token, matches[1].str());
          return;
        }
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
        // check if '&' or "of"
        // skip if delimiter
        // check if number
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

    // separated number (e.g. ` - 08`)
    {
      static constexpr auto is_dash = [](const Token& token) {
        return token.kind == TokenKind::Delimiter && token.delimiter_kind == DelimiterKind::Dash;
      };

      auto tokens = tokens_ | filter(is_dash);

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

  inline void search_volume_number() noexcept {
    static constexpr auto is_volume_keyword = [](const Token& token) {
      return token.keyword && token.keyword->kind == KeywordKind::Volume;
    };

    auto volume_token = std::ranges::find_if(tokens_, is_volume_keyword);

    // Check next token for a number
    if (auto token = find_next_token(volume_token, is_not_delimiter_token);
        token != tokens_.end()) {
      if (is_free_token(*token) && is_numeric_token(*token)) {
        add_element_from_token(ElementKind::VolumeNumber, *token);
        volume_token->element_kind = ElementKind::VolumeNumber;
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

    auto span = std::span(token_begin, token_end);

    // Trim delimiters and open brackets
    while (span.back().kind == TokenKind::Delimiter || span.back().kind == TokenKind::OpenBracket) {
      span = span.first(span.size() - 1);
    }

    // If the interval ends with an enclosed group (e.g. "Anime Title [Fansub]"),
    // move the upper endpoint back to the beginning of the group. We ignore
    // parentheses in order to keep certain groups (e.g. "(TV)") intact.
    //
    // @TODO

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
