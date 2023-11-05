#pragma once

#include <format>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "util.hpp"

// This is an incomplete implementation of the JSON format. It is only supposed to parse a single
// handcrafted document for testing purposes. Use at your own risk.
//
// Reference: https://www.json.org/

namespace anitomy::detail::json {

class Value final {
public:
  using string_t = std::string;
  using object_t = std::map<string_t, Value>;
  using array_t = std::vector<Value>;
  using value_t = std::variant<object_t, array_t, string_t, int, bool, nullptr_t>;

  enum Kind : size_t {
    Object,
    Array,
    String,
    Number,
    Boolean,
    Null,
  };

  explicit Value() = default;

  explicit Value(value_t&& value) : value_{std::move(value)} {
  }

  explicit Value(std::string_view input) : input_{input}, view_{input_}, value_{parse_value()} {
  }

  inline object_t& as_object() noexcept {
    static object_t empty_object;
    return holds(Kind::Object) ? std::get<object_t>(value_) : empty_object;
  }

  inline array_t& as_array() noexcept {
    static array_t empty_array;
    return holds(Kind::Array) ? std::get<array_t>(value_) : empty_array;
  }

  inline string_t as_string() const noexcept {
    return holds(Kind::String) ? std::get<string_t>(value_) : string_t{};
  }

  inline int as_number() const noexcept {
    return holds(Kind::Number) ? std::get<int>(value_) : 0;
  }

  inline bool as_bool() const noexcept {
    return holds(Kind::Boolean) ? std::get<bool>(value_) : false;
  }

  inline bool holds(const Kind kind) const noexcept {
    return value_.index() == kind;
  }

  inline const value_t& value() const noexcept {
    return value_;
  }

private:
  inline value_t parse_value() noexcept {
    const auto parse = [this]() -> value_t {
      if (view_.empty()) {
        return object_t{};
      }
      if (peek() == '{') {
        return parse_object();
      }
      if (peek() == '[') {
        return parse_array();
      }
      if (peek() == '"') {
        return parse_string();
      }
      if (is_digit(peek())) {
        return parse_number();
      }
      if (peek() == 't' || peek() == 'f') {
        return parse_boolean();
      }
      if (peek() == 'n') {
        return parse_null();
      }
      return nullptr;
    };

    skip_whitespace();
    value_t value = parse();
    skip_whitespace();
    return value;
  }

  inline object_t parse_object() noexcept {
    object_t object;

    skip('{');
    skip_whitespace();
    while (peek() != '}') {
      skip_whitespace();
      string_t name = parse_string();
      skip_whitespace();
      skip(':');
      object[name] = Value{parse_value()};
      skip(',');
      skip_whitespace();
    }
    skip('}');

    return object;
  }

  inline array_t parse_array() noexcept {
    array_t array;

    skip('[');
    skip_whitespace();
    while (peek() != ']') {
      skip_whitespace();
      array.emplace_back(parse_value());
      skip(',');
      skip_whitespace();
    }
    skip(']');

    return array;
  }

  inline string_t parse_string() noexcept {
    const auto parse = [this]() -> string_t {
      const auto is_string = [](const char ch) { return ch != '"'; };
      auto text = view_ | std::views::take_while(is_string);
      auto n = std::ranges::distance(text);
      return take(n);
    };

    skip('"');
    string_t string = parse();
    skip('"');
    return string;
  }

  inline int parse_number() noexcept {
    auto text = view_ | std::views::take_while(is_digit);
    auto n = std::ranges::distance(text);
    return to_int(take(n));
  }

  inline bool parse_boolean() noexcept {
    if (view_.starts_with("true")) {
      view_.remove_prefix(4);
      return true;
    }
    if (view_.starts_with("false")) {
      view_.remove_prefix(5);
      return false;
    }
    return false;
  }

  inline nullptr_t parse_null() noexcept {
    if (view_.starts_with("null")) {
      view_.remove_prefix(4);
    }
    return nullptr;
  }

  [[nodiscard]] constexpr char peek() const noexcept {
    return view_.front();
  }

  [[nodiscard]] constexpr std::string take(const size_t n = 1) noexcept {
    auto view = view_ | std::views::take(n);
    view_.remove_prefix(n);
    return std::string{view};
  }

  inline void skip(const char ch) noexcept {
    if (view_.starts_with(ch)) view_.remove_prefix(1);
  };

  inline void skip_whitespace() noexcept {
    const auto is_whitespace = [](const char ch) {
      return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };
    while (!view_.empty() && is_whitespace(view_.front())) {
      view_.remove_prefix(1);
    }
  }

  std::string input_;
  std::string_view view_;
  value_t value_;
};

[[nodiscard]] inline Value parse(std::string_view input) noexcept {
  return Value{input};
}

[[nodiscard]] inline std::string escape_string(std::string input) noexcept {
  static std::regex reserved(R"(["\\])");
  return std::regex_replace(input, reserved, R"(\$&)");
};

[[nodiscard]] inline std::string unescape_string(std::string input) noexcept {
  static std::regex reserved("\\\\([\"\\\\])");
  auto output = std::regex_replace(input, reserved, "$1");
  return output;
};

[[nodiscard]] inline std::string serialize(
    const std::vector<std::pair<std::string, std::string>>& items, bool pretty) noexcept {
  std::string output;

  output.push_back('{');

  for (auto it = items.begin(); it != items.end(); ++it) {
    const auto& [name, value] = *it;
    if (pretty) output.append("\n\t");
    output.append(std::format(R"("{}":)", escape_string(name)));
    if (pretty) output.push_back(' ');
    output.append(std::format(R"("{}")", escape_string(value)));
    if (it + 1 < items.end()) output.push_back(',');
  }

  if (pretty) output.push_back('\n');
  output.push_back('}');

  return output;
}

}  // namespace anitomy::detail::json
