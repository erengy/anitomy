#pragma once

#include <expected>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>

#include <anitomy/detail/json/value.hpp>
#include <anitomy/detail/util.hpp>

namespace anitomy::detail::json {

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
    static const auto parse = [this]() -> expected_t<value_t> {
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
      if (peek() == '-' || is_digit(peek())) {
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
    static const auto parse = [this]() -> string_t {
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

  [[nodiscard]] inline expected_t<value_t> parse_number() noexcept {
    static const std::regex pattern{R"(-?(?:0|[1-9]\d*)(\.\d+)?(?:[Ee][-+]?\d+)?)"};
    const std::string view{view_};
    std::smatch matches;

    if (!std::regex_match(view, matches, pattern)) return error();
    const size_t n = matches[0].length();

    if (matches[1].matched) {
      return to_float(take(n));
    } else {
      return to_int(take(n));
    }
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

}  // namespace anitomy::detail::json
