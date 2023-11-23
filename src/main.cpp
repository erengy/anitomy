#include <print>
#include <string>
#include <vector>

#include "../include/anitomy.hpp"
#include "../include/anitomy/cli.hpp"
#include "../include/anitomy/format.hpp"
#include "../include/anitomy/json.hpp"
#include "../include/anitomy/print.hpp"
#include "../include/anitomy/version.hpp"

namespace {

void print_usage() {
  std::println("anitomy {}", anitomy::version());
  std::println("Usage: anitomy [options...] <input>");
  std::println("Help: anitomy --help");
}

void print_help() {
  std::println("anitomy {}", anitomy::version());
  std::println("Usage: anitomy [options...] <input>");
  std::println("Options:");
  std::println("  --help             You are here");
  std::println("  --stdin            Use standard input");
  std::println("  --format=<format>  Set output format (`json` or `table`)");
  std::println("  --pretty           Pretty print JSON");
}

void print_elements(const std::vector<anitomy::Element>& elements) {
  using row_t = std::vector<std::string>;

  std::vector<row_t> rows;
  for (const auto& element : elements) {
    std::string kind{anitomy::detail::to_string(element.kind)};
    rows.emplace_back(row_t{kind, element.value});
  }

  anitomy::detail::print_table({"Element", "Value"}, rows);
}

void print_elements_json(const std::vector<anitomy::Element>& elements, bool pretty) {
  using namespace anitomy::detail;

  json::Value items{json::Value::object_t{}};
  for (const auto& element : elements) {
    items.as_object().emplace(to_string(element.kind), element.value);
  }

  std::print("{}", json::serialize(items, pretty));
}

bool is_trivial_token(const anitomy::detail::Token& token) noexcept {
  using enum anitomy::detail::TokenKind;
  switch (token.kind) {
    case OpenBracket:
    case CloseBracket:
    case Delimiter:
      return true;
    default:
      return false;
  };
}

void print_tokens(const std::vector<anitomy::detail::Token>& tokens, bool skip_trivial) {
  using anitomy::detail::to_string;
  using row_t = std::vector<std::string>;

  std::vector<row_t> rows;
  for (const auto& token : tokens) {
    if (skip_trivial && is_trivial_token(token)) continue;
    rows.emplace_back(row_t{
        std::string{to_string(token.kind)},
        std::string{token.keyword ? to_string(token.keyword->kind) : ""},
        std::string{token.element_kind ? to_string(*token.element_kind) : ""},
        token.value,
    });
  }

  anitomy::detail::print_table({"Token", "Keyword", "Element", "Value"}, rows);
}

void print_tokens_json(const std::vector<anitomy::detail::Token>& tokens, bool pretty,
                       bool skip_trivial) {
  using namespace anitomy::detail;

  json::Value items{json::Value::array_t{}};
  for (const auto& token : tokens) {
    if (skip_trivial && is_trivial_token(token)) continue;
    items.as_array().emplace_back(token.value);
  }

  std::print("{}", json::serialize(items, pretty));
}

}  // namespace

int main(int argc, char* argv[]) {
  const anitomy::detail::CommandLine cli{argc, argv};

  if (cli.contains("help")) {
    print_help();
    return 0;
  }
  if (cli.input().empty()) {
    print_usage();
    return 0;
  }

  const anitomy::Options options;
  anitomy::detail::Tokenizer tokenizer{cli.input()};
  tokenizer.tokenize(options);
  anitomy::detail::Parser parser{tokenizer.tokens()};
  parser.parse(options);

  const std::string output = cli.get("output", "elements");
  const std::string format = cli.get("format", "table");
  const bool pretty = cli.contains("pretty");
  const bool skip_trivial = cli.contains("skip-trivial");

  if (output == "elements") {
    if (format == "json") {
      print_elements_json(parser.elements(), pretty);
    } else {
      print_elements(parser.elements());
    }
  } else if (output == "tokens") {
    if (format == "json") {
      print_tokens_json(parser.tokens(), pretty, skip_trivial);
    } else {
      print_tokens(parser.tokens(), skip_trivial);
    }
  }

  return 0;
}
