#pragma once

#include <format>
#include <string_view>

#include <anitomy/detail/format.hpp>
#include <anitomy/element.hpp>

template <>
struct std::formatter<anitomy::ElementKind> : std::formatter<std::string_view> {
  auto format(anitomy::ElementKind value, std::format_context& ctx) const {
    auto view = anitomy::detail::to_string(value);
    return std::formatter<std::string_view>::format(view, ctx);
  }
};
