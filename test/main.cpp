#include <array>
#include <cassert>
#include <cmath>
#include <format>
#include <limits>
#include <map>
#include <print>
#include <vector>

#include <anitomy.hpp>
#include <anitomy/detail/cli.hpp>
#include <anitomy/detail/format.hpp>
#include <anitomy/detail/json.hpp>
#include <anitomy/detail/unicode.hpp>

namespace {

void test_cli() {
  using namespace anitomy::detail;

  {
    CommandLine cl{{"anitomy"}};
    assert(cl.input().empty());
  }
  {
    CommandLine cl{{"anitomy", "test"}};
    assert(cl.input() == "test");
  }
  {
    CommandLine cl{{"anitomy", "\"test input\""}};
    assert(cl.input() == "test input");
  }
  {
    CommandLine cl{{"anitomy", "--help", "test"}};
    assert(cl.contains("help"));
    assert(cl.input() == "test");
  }
  {
    CommandLine cl{{"anitomy", "--format=json", "test"}};
    assert(cl.get("format") == "json");
    assert(cl.input() == "test");
  }
}

void test_json() {
  using namespace anitomy::detail;

  static constexpr auto is_equal = [](const float a, const float b) {
    return std::fabs(a - b) <= std::numeric_limits<float>::epsilon();
  };

  {
    auto value = json::parse("");
    assert(value.is_object());
    assert(value.as_object().empty());
    assert(json::serialize(value) == "{}");
  }
  {
    auto value = json::parse("invalid");
    assert(value.is_object());
    assert(json::serialize(value) == "{}");
  }
  {
    auto value = json::parse("{}");
    assert(value.is_object());
    assert(value.as_object().empty());
    assert(json::serialize(value) == "{}");
  }
  {
    auto value = json::parse("[]");
    assert(value.is_array());
    assert(value.as_array().empty());
    assert(json::serialize(value) == "[]");
  }
  {
    const auto str = R"("test")";
    auto value = json::parse(str);
    assert(value.is_string());
    assert(value.as_string() == "test");
    assert(json::serialize(value) == str);
  }
  {
    const auto str = R"("\"test\"")";
    auto value = json::parse(str);
    assert(value.is_string());
    assert(value.as_string() == R"("test")");
    assert(json::serialize(value) == str);
  }
  {
    const auto str = R"("test\\test")";
    auto value = json::parse(str);
    assert(value.is_string());
    assert(value.as_string() == R"(test\test)");
    assert(json::serialize(value) == str);
  }
  {
    auto value = json::parse("123");
    assert(value.is_integer());
    assert(value.as_integer() == 123);
    assert(json::serialize(value) == "123");
  }
  {
    auto value = json::parse("-123");
    assert(value.is_integer());
    assert(value.as_integer() == -123);
    assert(json::serialize(value) == "-123");
  }
  {
    auto value = json::parse("123.45");
    assert(value.is_float());
    assert(is_equal(value.as_float(), 123.45f));
    assert(json::serialize(value) == "123.45");
  }
  {
    auto value = json::parse("-123.45");
    assert(value.is_float());
    assert(is_equal(value.as_float(), -123.45f));
    assert(json::serialize(value) == "-123.45");
  }
  {
    auto value = json::parse("-123.45e-2");
    assert(value.is_float());
    assert(is_equal(value.as_float(), -1.2345f));
    auto a = json::serialize(value);
    assert(json::serialize(value) == "-1.2345");
  }
  {
    auto value = json::parse("-123.45E+1");
    assert(value.is_float());
    assert(is_equal(value.as_float(), -1234.5f));
    auto a = json::serialize(value);
    assert(json::serialize(value) == "-1234.5");
  }
  {
    auto value = json::parse("true");
    assert(value.is_bool());
    assert(value.as_bool() == true);
    assert(json::serialize(value) == "true");
  }
  {
    auto value = json::parse("false");
    assert(value.is_bool());
    assert(value.as_bool() == false);
    assert(json::serialize(value) == "false");
  }
  {
    auto value = json::parse("null");
    assert(value.is_null());
    assert(json::serialize(value) == "null");
  }
  {
    auto value = json::parse("nullz");
    assert(value.is_null());
    assert(json::serialize(value) == "null");
  }
  {
    const auto str = R"({"episode":"01","title":"Title"})";
    auto value = json::parse(str);
    assert(value.as_object()["episode"].as_string() == "01");
    assert(value.as_object()["title"].as_string() == "Title");
    assert(json::serialize(value) == str);
  }
  {
    const auto str = R"({"a":["b",{"c":"d"}]})";
    auto value = json::parse(str);
    assert(value.as_object()["a"].as_array()[0].as_string() == "b");
    assert(value.as_object()["a"].as_array()[1].as_object()["c"].as_string() == "d");
    assert(json::serialize(value) == str);
  }
}

void test_parser() {
  using namespace anitomy::detail;

  anitomy::Options options;

  {
    Tokenizer t{""};
    t.tokenize(options);
    Parser p{t.tokens()};
    p.parse(options);
    assert(p.tokens().empty());
    assert(p.elements().empty());
  }
}

void test_tokenizer() {
  using namespace anitomy::detail;

  anitomy::Options options;

  {
    Tokenizer t{""};
    t.tokenize(options);
    assert(t.tokens().empty());
  }
  {
    Tokenizer t{
        "[TaigaSubs]_Toradora!_(2008)_-_01v2_-_Tiger_and_Dragon_[1280x720_H.264_FLAC][1234ABCD]"};
    t.tokenize(options);
    // clang-format off
    const std::vector<std::pair<TokenKind, std::string>> tokens{
        {TokenKind::OpenBracket, "["},
        {TokenKind::Text, "TaigaSubs"},
        {TokenKind::CloseBracket, "]"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Text, "Toradora!"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::OpenBracket, "("},
        {TokenKind::Text, "2008"},
        {TokenKind::CloseBracket, ")"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Delimiter, "-"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Text, "01v2"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Delimiter, "-"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Text, "Tiger"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Text, "and"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Text, "Dragon"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::OpenBracket, "["},
        {TokenKind::Text, "1280x720"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Keyword, "H.264"},
        {TokenKind::Delimiter, "_"},
        {TokenKind::Keyword, "FLAC"},
        {TokenKind::CloseBracket, "]"},
        {TokenKind::OpenBracket, "["},
        {TokenKind::Text, "1234ABCD"},
        {TokenKind::CloseBracket, "]"},
    };
    // clang-format on
    assert(t.tokens().size() == tokens.size());
    for (size_t i = 0; i < tokens.size(); ++i) {
      assert(t.tokens()[i].kind == tokens[i].first);
      assert(t.tokens()[i].value == tokens[i].second);
    }
  }
}

void test_unicode() {
  using namespace anitomy::detail::unicode;

  assert(is_scalar_value(0));
  assert(is_scalar_value(0xD7FF));
  assert(is_scalar_value(0xD800) == false);
  assert(is_scalar_value(0xDFFF) == false);
  assert(is_scalar_value(0xE000));
  assert(is_scalar_value(0x10FFFF));
  assert(is_scalar_value(0x110000) == false);

  assert(is_surrogate(0) == false);
  assert(is_surrogate(0xD7FF) == false);
  assert(is_surrogate(0xD800));
  assert(is_surrogate(0xDFFF));
  assert(is_surrogate(0xE000) == false);

  assert(utf8::is_continuation(0b00000000) == false);
  assert(utf8::is_continuation(0b01111111) == false);
  assert(utf8::is_continuation(0b10000000));
  assert(utf8::is_continuation(0b10111111));
  assert(utf8::is_continuation(0b11000000) == false);
  assert(utf8::is_continuation(0b11111111) == false);

  assert(utf8::is_invalid(0x00) == false);
  assert(utf8::is_invalid(0xBF) == false);
  assert(utf8::is_invalid(0xC0));
  assert(utf8::is_invalid(0xC1));
  assert(utf8::is_invalid(0xC2) == false);
  assert(utf8::is_invalid(0xF4) == false);
  for (int b = 0xF5; b <= 0xFF; ++b) {
    assert(utf8::is_invalid(b));
  }

  assert(utf8::sequence_length(0b00000000) == 1);
  assert(utf8::sequence_length(0b00001000) == 1);
  assert(utf8::sequence_length(0b00010000) == 1);
  assert(utf8::sequence_length(0b00011000) == 1);
  assert(utf8::sequence_length(0b00100000) == 1);
  assert(utf8::sequence_length(0b00101000) == 1);
  assert(utf8::sequence_length(0b00110000) == 1);
  assert(utf8::sequence_length(0b00111000) == 1);
  assert(utf8::sequence_length(0b01000000) == 1);
  assert(utf8::sequence_length(0b01001000) == 1);
  assert(utf8::sequence_length(0b01010000) == 1);
  assert(utf8::sequence_length(0b01011000) == 1);
  assert(utf8::sequence_length(0b01100000) == 1);
  assert(utf8::sequence_length(0b01101000) == 1);
  assert(utf8::sequence_length(0b01110000) == 1);
  assert(utf8::sequence_length(0b01111000) == 1);
  assert(utf8::sequence_length(0b10000000) == 0);
  assert(utf8::sequence_length(0b10001000) == 0);
  assert(utf8::sequence_length(0b10010000) == 0);
  assert(utf8::sequence_length(0b10011000) == 0);
  assert(utf8::sequence_length(0b10100000) == 0);
  assert(utf8::sequence_length(0b10101000) == 0);
  assert(utf8::sequence_length(0b10110000) == 0);
  assert(utf8::sequence_length(0b10111000) == 0);
  assert(utf8::sequence_length(0b11000000) == 2);
  assert(utf8::sequence_length(0b11001000) == 2);
  assert(utf8::sequence_length(0b11010000) == 2);
  assert(utf8::sequence_length(0b11011000) == 2);
  assert(utf8::sequence_length(0b11100000) == 3);
  assert(utf8::sequence_length(0b11101000) == 3);
  assert(utf8::sequence_length(0b11110000) == 4);
  assert(utf8::sequence_length(0b11111000) == 0);

  assert(utf8::encode(0x00007F) == "\x7F");
  assert(utf8::encode(0x000080) == "\xC2\x80");
  assert(utf8::encode(0x000081) == "\xC2\x81");
  assert(utf8::encode(0x000082) == "\xC2\x82");
  assert(utf8::encode(0x00D7FF) == "\xED\x9F\xBF");
  assert(utf8::encode(0x00D800) == "\xEF\xBF\xBD");
  assert(utf8::encode(0x00DFFF) == "\xEF\xBF\xBD");
  assert(utf8::encode(0x00E000) == "\xEE\x80\x80");
  assert(utf8::encode(0x00FFFF) == "\xEF\xBF\xBF");
  assert(utf8::encode(0x010000) == "\xF0\x90\x80\x80");
  assert(utf8::encode(0x10FFFF) == "\xF4\x8F\xBF\xBF");
  assert(utf8::encode(0x11FFFF) == "\xEF\xBF\xBD");

  const auto utf8_decode = [](std::string_view s) {
    const auto result = utf8::decode(s.begin(), s.end());
    return result.code_point;
  };
  assert(utf8_decode("\x01") == 0x000001);
  assert(utf8_decode("\x7F") == 0x00007F);
  assert(utf8_decode("\xC2\x80") == 0x000080);
  assert(utf8_decode("\xC2\x81") == 0x000081);
  assert(utf8_decode("\xC2\x82") == 0x000082);
  assert(utf8_decode("\xED\x9F\xBF") == 0x00D7FF);
  assert(utf8_decode("\xED\x9F\xC0") == replacement_character);  // 0x00D800
  assert(utf8_decode("\xED\x9F\xC1") == replacement_character);  // 0x00DFFF
  assert(utf8_decode("\xEE\x80\x80") == 0x00E000);
  assert(utf8_decode("\xEF\xBF\xBF") == 0x00FFFF);
  assert(utf8_decode("\xF0\x90\x80\x80") == 0x010000);
  assert(utf8_decode("\xF4\x8F\xBF\xBF") == 0x10FFFF);
  assert(utf8_decode("\xF4\x8F\xBF\xC0") == replacement_character);  // 0x110000

  const auto utf32_decode = [](std::u32string_view s) {
    const auto result = utf32::decode(s.begin(), s.end());
    return result.code_point;
  };
  assert(utf32_decode(U"\x000001") == 0x000001);
  assert(utf32_decode(U"\x00D7FF") == 0x00D7FF);
  assert(utf32_decode(U"\x00D800") == replacement_character);
  assert(utf32_decode(U"\x00DFFF") == replacement_character);
  assert(utf32_decode(U"\x00E000") == 0x00E000);
  assert(utf32_decode(U"\x10FFFF") == 0x10FFFF);
  assert(utf32_decode(U"\x110000") == replacement_character);
  assert(utf32_decode(U"\x111111") == replacement_character);
}

void test_util() {
  using namespace anitomy::detail;

  assert(from_ordinal_number("") == "");
  assert(from_ordinal_number("1st") == "1");
  assert(from_ordinal_number("9th") == "9");
  assert(from_ordinal_number("0th") == "");
  assert(from_ordinal_number("First") == "1");
  assert(from_ordinal_number("first") == "");

  assert(from_roman_number("") == "");
  assert(from_roman_number("I") == "");
  assert(from_roman_number("II") == "2");
  assert(from_roman_number("III") == "3");
  assert(from_roman_number("IV") == "4");
  assert(from_roman_number("V") == "");

  for (char ch = 'A'; ch <= 'Z'; ++ch) {
    assert(is_alpha(ch));
  }
  for (char ch = 'z'; ch <= 'z'; ++ch) {
    assert(is_alpha(ch));
  }

  for (char ch = '0'; ch <= '9'; ++ch) {
    assert(is_digit(ch));
  }

  for (char ch = '0'; ch <= '9'; ++ch) {
    assert(is_xdigit(ch));
  }
  for (char ch = 'A'; ch <= 'F'; ++ch) {
    assert(is_xdigit(ch));
  }
  for (char ch = 'a'; ch <= 'f'; ++ch) {
    assert(is_xdigit(ch));
  }

  assert(to_int("") == 0);
  assert(to_int("-2147483648") == std::numeric_limits<int>::min());
  assert(to_int("-1") == -1);
  assert(to_int("0") == 0);
  assert(to_int("1") == 1);
  assert(to_int("2147483647") == std::numeric_limits<int>::max());
  assert(to_int("123abc") == 123);
  assert(to_int("abc123") == 0);

  assert(to_lower('A') == 'a');
  assert(to_lower('Z') == 'z');
  assert(to_lower('a') == 'a');
  assert(to_lower('1') == '1');
  assert(to_lower('\0') == '\0');
}

void test_data() {
  using namespace anitomy::detail;

  std::string file;
  if (!read_file("data.json", file)) assert(0 && "Cannot read test data");

  auto data = json::parse(file);
  if (!data.is_array()) assert(0 && "Invalid test data");

  const auto make_element_map = [](const std::vector<anitomy::Element>& elements) {
    std::map<std::string, std::vector<std::string>> map;
    for (const auto& element : elements) {
      map[std::string{anitomy::detail::to_string(element.kind)}].push_back(element.value);
    };
    return map;
  };

  const auto get_value_vector = [](anitomy::detail::json::Value& arr) {
    std::vector<std::string> values;
    for (auto& value : arr.as_array()) {
      values.emplace_back(value.as_string());
    }
    return values;
  };

  const auto vector_to_string = [](const std::vector<std::string>& v) {
    std::string output;
    for (const auto& s : v) {
      if (!output.empty()) output.append(", ");
      output.append(s);
    }
    return output;
  };

  const auto print_error = [](std::string_view input, std::string_view name,
                              std::string_view expected_value, std::string_view value) {
    std::println("Input:    `{}`", input);
    std::println("Element:  `{}`", name);
    std::println("Expected: `{}`", expected_value);
    std::println("Got:      `{}`", value);
    std::println("");
  };

  for (auto& item : data.as_array()) {
    if (!item.is_object()) assert(0 && "Invalid test data");
    auto& map = item.as_object();

    if (!map.contains("input")) assert(0 && "Invalid test data");
    const auto input = map["input"].as_string();
    auto elements = make_element_map(anitomy::parse(input));

    if (!map.contains("output")) assert(0 && "Invalid test data");
    auto& output = map["output"].as_object();

    for (auto& [name, expected_value] : output) {
      if (expected_value.is_string()) {
        if (elements[name].size() == 1 && elements[name].front() == expected_value.as_string()) {
          continue;
        }
        print_error(input, name, expected_value.as_string(),
                    !elements[name].empty() ? elements[name].front() : "");
      } else if (expected_value.is_array()) {
        const auto expected_values = get_value_vector(expected_value);
        if (elements[name] == expected_values) continue;
        print_error(input, name, vector_to_string(expected_values),
                    vector_to_string(elements[name]));
      }
    }
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string_view arg{argc == 2 ? argv[1] : ""};

  if (arg == "--test-data") {
    test_data();
  } else {
    test_cli();
    test_json();
    test_parser();
    test_tokenizer();
    test_unicode();
    test_util();
    std::println("Passed all tests!");
  }

  return 0;
}
