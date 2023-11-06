#include <array>
#include <cassert>
#include <format>
#include <limits>
#include <map>
#include <print>
#include <vector>

#include "../include/anitomy.hpp"
#include "../include/anitomy/cli.hpp"
#include "../include/anitomy/format.hpp"
#include "../include/anitomy/json.hpp"
#include "../include/anitomy/unicode.hpp"

namespace {

void test_cli() {
  using namespace anitomy::detail;

  {
    std::vector<char*> params{(char*)"anitomy"};
    CommandLine cl{params};
    assert(cl.input().empty());
  }
  {
    std::vector<char*> params{(char*)"anitomy", (char*)"test"};
    CommandLine cl{params};
    assert(cl.input() == "test");
  }
  {
    std::vector<char*> params{(char*)"anitomy", (char*)"\"test input\""};
    CommandLine cl{params};
    assert(cl.input() == "test input");
  }
  {
    std::vector<char*> params{(char*)"anitomy", (char*)"--help", (char*)"test"};
    CommandLine cl{params};
    assert(cl.contains("help"));
    assert(cl.input() == "test");
  }
  {
    std::vector<char*> params{(char*)"anitomy", (char*)"--format=json", (char*)"test"};
    CommandLine cl{params};
    assert(cl.get("format") == "json");
    assert(cl.input() == "test");
  }
}

void test_json() {
  using namespace anitomy::detail;

  assert(json::escape_string("") == "");
  assert(json::escape_string("test") == "test");
  assert(json::escape_string(R"("test")") == R"(\"test\")");
  assert(json::escape_string(R"("test\test")") == R"(\"test\\test\")");

  assert(json::unescape_string("") == "");
  assert(json::unescape_string("test") == "test");
  assert(json::unescape_string(R"(\"test\")") == R"("test")");
  assert(json::unescape_string(R"(\"test\\test\")") == R"("test\test")");

  {
    auto value = json::parse("{}");
    assert(value.holds(json::Value::Object));
    assert(value.as_object().empty());
  }
  {
    auto value = json::parse("[]");
    assert(value.holds(json::Value::Array));
    assert(value.as_array().empty());
  }
  {
    const auto str = R"({"anime_title":"Title","episode_number":"01"})";
    auto value = json::parse(str);
    assert(value.holds(json::Value::Object));
    assert(value.as_object()["anime_title"].as_string() == "Title");
    assert(value.as_object()["episode_number"].as_string() == "01");
  }

  {
    const std::vector<std::pair<std::string, std::string>> items{};
    assert(json::serialize(items, false) == "{}");
  }
  {
    const std::vector<std::pair<std::string, std::string>> items{
        {"anime_title", "Title"},
        {"episode_number", "01"},
    };
    const auto str = R"({"anime_title":"Title","episode_number":"01"})";
    assert(json::serialize(items, false) == str);
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

}  // namespace

int main() {
  test_cli();
  test_json();
  test_parser();
  test_tokenizer();
  test_unicode();
  test_util();

  std::println("Passed all tests!");

  return 0;
}