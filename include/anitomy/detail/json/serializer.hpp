#pragma once

#include <format>
#include <regex>
#include <string>

#include <anitomy/detail/json/value.hpp>

namespace anitomy::detail::json {

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
      case Value::Integer:
        serialize_integer(value.as_integer(), output);
        break;
      case Value::Float:
        serialize_float(value.as_float(), output);
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

  static inline void serialize_integer(const int value, std::string& output) noexcept {
    output.append(std::format("{}", value));
  }

  static inline void serialize_float(const float value, std::string& output) noexcept {
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
    static const std::regex reserved{R"(["\\])"};
    return std::regex_replace(input, reserved, R"(\$&)");
  };

  int indentation_ = 0;
  bool pretty_ = false;
  Value value_;
};

}  // namespace anitomy::detail::json
