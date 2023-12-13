#pragma once

#include <optional>
#include <ranges>
#include <tuple>
#include <vector>

#include "../element.hpp"
#include "../token.hpp"

namespace anitomy::detail {

inline std::optional<Element> parse_anime_year(std::vector<Token>& tokens_) noexcept {
  using namespace std::views;
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
    auto& token = std::get<1>(tokens.front());
    token.element_kind = ElementKind::AnimeYear;
    return Element{
        .kind = ElementKind::AnimeYear,
        .value = token.value,
    };
  }

  return std::nullopt;
}

}  // namespace anitomy::detail
