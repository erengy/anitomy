#pragma once

#include <expected>
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

  explicit Value(value_t&& value) : value_{value} {
  }

  [[nodiscard]] inline object_t& as_object() noexcept {
    static object_t empty_object;
    return holds(Kind::Object) ? std::get<object_t>(value_) : empty_object;
  }

  [[nodiscard]] inline array_t& as_array() noexcept {
    static array_t empty_array;
    return holds(Kind::Array) ? std::get<array_t>(value_) : empty_array;
  }

  [[nodiscard]] inline string_t as_string() const noexcept {
    return holds(Kind::String) ? std::get<string_t>(value_) : string_t{};
  }

  [[nodiscard]] inline int as_number() const noexcept {
    return holds(Kind::Number) ? std::get<int>(value_) : 0;
  }

  [[nodiscard]] inline bool as_bool() const noexcept {
    return holds(Kind::Boolean) ? std::get<bool>(value_) : false;
  }

  [[nodiscard]] inline bool holds(const Kind kind) const noexcept {
    return value_.index() == kind;
  }

  [[nodiscard]] inline Kind kind() const noexcept {
    return static_cast<Kind>(value_.index());
  }

private:
  value_t value_;
};

class Parser final {
public:
  using string_t = Value::string_t;
  using object_t = Value::object_t;
  using array_t = Value::array_t;
  using value_t = Value::value_t;

  enum class Error {};

  template <typename T>
  using expected_t = std::expected<T, Error>;

  explicit Parser(std::string_view input) : input_{input}, view_{input_} {
  }

  [[nodiscard]] inline Value parse() noexcept {
    auto value = parse_value();
    return Value{value ? std::move(*value) : object_t{}};
  }

private:
  [[nodiscard]] static inline std::unexpected<Error> error() noexcept {
    return std::unexpected{Error{}};
  }

  [[nodiscard]] inline expected_t<value_t> parse_value() noexcept {
    const auto parse = [this]() -> expected_t<value_t> {
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
      return error();
    };

    skip_whitespace();
    auto value = parse();
    skip_whitespace();
    return value;
  }

  [[nodiscard]] inline expected_t<object_t> parse_object() noexcept {
    object_t object;

    if (!expect('{')) return error();
    skip_whitespace();
    while (!view_.empty() && peek() != '}') {
      skip_whitespace();
      auto name = parse_string();
      if (!name) return error();
      skip_whitespace();
      if (!expect(':')) return error();
      auto value = parse_value();
      if (!value) return error();
      object.emplace(*name, std::move(*value));
      expect(',');
      skip_whitespace();
    }
    if (!expect('}')) return error();

    return object;
  }

  [[nodiscard]] inline expected_t<array_t> parse_array() noexcept {
    array_t array;

    if (!expect('[')) return error();
    skip_whitespace();
    while (!view_.empty() && peek() != ']') {
      skip_whitespace();
      auto value = parse_value();
      if (!value) return error();
      array.emplace_back(std::move(*value));
      expect(',');
      skip_whitespace();
    }
    if (!expect(']')) return error();

    return array;
  }

  [[nodiscard]] inline expected_t<string_t> parse_string() noexcept {
    const auto parse = [this]() -> string_t {
      const auto is_string = [](const char ch) { return ch != '"'; };
      auto text = view_ | std::views::take_while(is_string);
      auto n = std::ranges::distance(text);
      return unescape_string(take(n));
    };

    if (!expect('"')) return error();
    string_t string = parse();
    if (!expect('"')) return error();
    return string;
  }

  [[nodiscard]] inline expected_t<int> parse_number() noexcept {
    auto text = view_ | std::views::take_while(is_digit);
    auto n = std::ranges::distance(text);
    if (!n) return error();
    return to_int(take(n));
  }

  [[nodiscard]] inline expected_t<bool> parse_boolean() noexcept {
    if (view_.starts_with("true")) {
      view_.remove_prefix(4);
      return true;
    }
    if (view_.starts_with("false")) {
      view_.remove_prefix(5);
      return false;
    }
    return error();
  }

  [[nodiscard]] inline expected_t<nullptr_t> parse_null() noexcept {
    if (view_.starts_with("null")) {
      view_.remove_prefix(4);
      return nullptr;
    }
    return error();
  }

  [[nodiscard]] constexpr char peek() const noexcept {
    return view_.front();
  }

  [[nodiscard]] constexpr std::string take(const size_t n = 1) noexcept {
    auto view = view_ | std::views::take(n);
    view_.remove_prefix(n);
    return std::string{view};
  }

  inline expected_t<char> expect(const char ch) noexcept {
    if (!view_.starts_with(ch)) return error();
    view_.remove_prefix(1);
    return ch;
  };

  inline void skip_whitespace() noexcept {
    const auto is_whitespace = [](const char ch) {
      return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };
    while (!view_.empty() && is_whitespace(view_.front())) {
      view_.remove_prefix(1);
    }
  }

  [[nodiscard]] static inline std::string unescape_string(std::string input) noexcept {
    static std::regex reserved("\\\\([\"\\\\])");
    auto output = std::regex_replace(input, reserved, "$1");
    return output;
  };

  std::string input_;
  std::string_view view_;
};

class Serializer final {
public:
  using string_t = Value::string_t;
  using object_t = Value::object_t;
  using array_t = Value::array_t;
  using value_t = Value::value_t;

  explicit Serializer(const Value& value, bool pretty) : value_{value}, pretty_{pretty} {
  }

  [[nodiscard]] inline std::string serialize() noexcept {
    std::string output;
    serialize_value(value_, output);
    return output;
  }

private:
  inline void serialize_value(Value& value, std::string& output) noexcept {
    switch (value.kind()) {
      case Value::Object:
        serialize_object(value.as_object(), output);
        break;
      case Value::Array:
        serialize_array(value.as_array(), output);
        break;
      case Value::String:
        serialize_string(value.as_string(), output);
        break;
      case Value::Number:
        serialize_number(value.as_number(), output);
        break;
      case Value::Boolean:
        serialize_boolean(value.as_bool(), output);
        break;
      case Value::Null:
        serialize_null(output);
        break;
    }
  }

  inline void serialize_object(object_t& object, std::string& output) noexcept {
    output.push_back('{');
    if (!object.empty()) {
      if (pretty_) {
        output.push_back('\n');
        indentation_ += 1;
      }
      for (auto it = object.begin(); it != object.end(); ++it) {
        auto& [name, value] = *it;
        if (pretty_) serialize_indentation(output);
        serialize_string(name, output);
        output.push_back(':');
        if (pretty_) output.push_back(' ');
        serialize_value(value, output);
        if (std::next(it) != object.end()) output.push_back(',');
        if (pretty_) output.push_back('\n');
      }
      if (pretty_) {
        indentation_ -= 1;
      }
    }
    output.push_back('}');
  }

  inline void serialize_array(array_t& array, std::string& output) noexcept {
    output.push_back('[');
    if (!array.empty()) {
      if (pretty_) {
        output.push_back('\n');
        indentation_ += 1;
      }
      for (auto it = array.begin(); it != array.end(); ++it) {
        if (pretty_) serialize_indentation(output);
        serialize_value(*it, output);
        if (std::next(it) != array.end()) output.push_back(',');
        if (pretty_) output.push_back('\n');
      }
      if (pretty_) {
        indentation_ -= 1;
      }
    }
    output.push_back(']');
  }

  static inline void serialize_string(const string_t& value, std::string& output) noexcept {
    output.append(std::format(R"("{}")", escape_string(value)));
  }

  static inline void serialize_number(const int value, std::string& output) noexcept {
    output.append(std::format("{}", value));
  }

  static inline void serialize_boolean(const bool value, std::string& output) noexcept {
    output.append(value ? "true" : "false");
  }

  static inline void serialize_null(std::string& output) noexcept {
    output.append("null");
  }

  inline void serialize_indentation(std::string& output) const noexcept {
    output.append(std::format("{:<{}}", "", indentation_ * 2));
  }

  [[nodiscard]] static inline std::string escape_string(std::string input) noexcept {
    static std::regex reserved(R"(["\\])");
    return std::regex_replace(input, reserved, R"(\$&)");
  };

  int indentation_ = 0;
  bool pretty_ = false;
  Value value_;
};

[[nodiscard]] inline Value parse(std::string_view input) noexcept {
  Parser parser{input};
  return parser.parse();
}

[[nodiscard]] inline std::string serialize(const Value& value, bool pretty = false) noexcept {
  Serializer serializer{value, pretty};
  return serializer.serialize();
}

}  // namespace anitomy::detail::json
