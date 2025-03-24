#pragma once

#include <algorithm>
#include <functional>
#include <span>

#include <anitomy/detail/token.hpp>

namespace anitomy::detail {

using token_predicate_t = std::function<bool(const Token&)>;

template <typename Container, typename It = Container::iterator>
inline It find_prev_token(Container& container, It it, token_predicate_t predicate) noexcept {
  if (it == container.begin()) return container.end();
  auto [token, _] = std::ranges::find_last_if(container.begin(), it, predicate);
  return token == it ? container.end() : token;
}

template <typename Container, typename It = Container::iterator>
inline It find_next_token(Container& container, It it, token_predicate_t predicate) noexcept {
  if (it == container.end()) return container.end();
  return std::ranges::find_if(std::next(it), container.end(), predicate);
}

template <typename It>
inline It find_next_token(It first, It last, token_predicate_t predicate) noexcept {
  if (first == last) return last;
  return std::ranges::find_if(std::next(first), last, predicate);
}

}  // namespace anitomy::detail
