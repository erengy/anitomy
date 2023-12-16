#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

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
    return is_object() ? std::get<object_t>(value_) : empty_object;
  }

  [[nodiscard]] inline array_t& as_array() noexcept {
    static array_t empty_array;
    return is_array() ? std::get<array_t>(value_) : empty_array;
  }

  [[nodiscard]] inline string_t as_string() const noexcept {
    return is_string() ? std::get<string_t>(value_) : string_t{};
  }

  [[nodiscard]] inline int as_number() const noexcept {
    return is_number() ? std::get<int>(value_) : 0;
  }

  [[nodiscard]] inline bool as_bool() const noexcept {
    return is_bool() ? std::get<bool>(value_) : false;
  }

  [[nodiscard]] inline bool is_object() const noexcept {
    return holds(Kind::Object);
  }

  [[nodiscard]] inline bool is_array() const noexcept {
    return holds(Kind::Array);
  }

  [[nodiscard]] inline bool is_string() const noexcept {
    return holds(Kind::String);
  }

  [[nodiscard]] inline bool is_number() const noexcept {
    return holds(Kind::Number);
  }

  [[nodiscard]] inline bool is_bool() const noexcept {
    return holds(Kind::Boolean);
  }

  [[nodiscard]] inline bool is_null() const noexcept {
    return holds(Kind::Null);
  }

  [[nodiscard]] inline Kind kind() const noexcept {
    return static_cast<Kind>(value_.index());
  }

private:
  [[nodiscard]] inline bool holds(const Kind kind) const noexcept {
    return value_.index() == kind;
  }

  value_t value_;
};

}  // namespace anitomy::detail::json
