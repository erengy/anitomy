#pragma once

#include <string>
#include <string_view>

#include <anitomy/detail/json/parser.hpp>
#include <anitomy/detail/json/serializer.hpp>

// This is an incomplete implementation of the JSON format. It is only supposed to parse a single
// handcrafted document for testing purposes. Use at your own risk.
//
// Reference: https://www.json.org/

namespace anitomy::detail::json {

[[nodiscard]] inline Value parse(std::string_view input) noexcept {
  Parser parser{input};
  return parser.parse();
}

[[nodiscard]] inline std::string serialize(const Value& value, bool pretty = false) noexcept {
  Serializer serializer{value, pretty};
  return serializer.serialize();
}

}  // namespace anitomy::detail::json
