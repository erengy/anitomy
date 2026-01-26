#pragma once

#include <algorithm>
#include <ranges>
#include <utility>

// This file contains `std::ranges::starts_with` and `std::ranges::ends_with`
// taken from the "Possible implementation" code found on cppreference.com.
// https://en.cppreference.com/w/cpp/algorithm/ranges/starts_with.html
// https://en.cppreference.com/w/cpp/algorithm/ranges/ends_with.html

namespace std::ranges {
struct starts_with_fn {
  template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
            std::sentinel_for<I2> S2, class Pred = ranges::equal_to, class Proj1 = std::identity,
            class Proj2 = std::identity>
    requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
  constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                            Proj1 proj1 = {}, Proj2 proj2 = {}) const {
    return ranges::mismatch(std::move(first1), last1, std::move(first2), last2, std::move(pred),
                            std::move(proj1), std::move(proj2))
               .in2 == last2;
  }

  template <ranges::input_range R1, ranges::input_range R2, class Pred = ranges::equal_to,
            class Proj1 = std::identity, class Proj2 = std::identity>
    requires std::indirectly_comparable<ranges::iterator_t<R1>, ranges::iterator_t<R2>, Pred, Proj1,
                                        Proj2>
  constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
                            Proj2 proj2 = {}) const {
    return (*this)(ranges::begin(r1), ranges::end(r1), ranges::begin(r2), ranges::end(r2),
                   std::move(pred), std::move(proj1), std::move(proj2));
  }
};

inline constexpr starts_with_fn starts_with{};

struct ends_with_fn {
  template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
            std::sentinel_for<I2> S2, class Pred = ranges::equal_to, class Proj1 = std::identity,
            class Proj2 = std::identity>
    requires(std::forward_iterator<I1> || std::sized_sentinel_for<S1, I1>) &&
            (std::forward_iterator<I2> || std::sized_sentinel_for<S2, I2>) &&
            std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
  constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                            Proj1 proj1 = {}, Proj2 proj2 = {}) const {
    const auto n1 = ranges::distance(first1, last1);
    const auto n2 = ranges::distance(first2, last2);
    if (n1 < n2) return false;
    ranges::advance(first1, n1 - n2);
    return ranges::equal(std::move(first1), last1, std::move(first2), last2, pred, proj1, proj2);
  }

  template <ranges::input_range R1, ranges::input_range R2, class Pred = ranges::equal_to,
            class Proj1 = std::identity, class Proj2 = std::identity>
    requires(ranges::forward_range<R1> || ranges::sized_range<R1>) &&
            (ranges::forward_range<R2> || ranges::sized_range<R2>) &&
            std::indirectly_comparable<ranges::iterator_t<R1>, ranges::iterator_t<R2>, Pred, Proj1,
                                       Proj2>
  constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
                            Proj2 proj2 = {}) const {
    const auto n1 = ranges::distance(r1);
    const auto n2 = ranges::distance(r2);
    if (n1 < n2) return false;
    return ranges::equal(views::drop(ranges::ref_view(r1), n1 - static_cast<decltype(n1)>(n2)), r2,
                         pred, proj1, proj2);
  }
};

inline constexpr ends_with_fn ends_with{};
}  // namespace std::ranges
