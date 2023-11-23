#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "../include/anitomy.hpp"
#include "../include/anitomy/cli.hpp"
#include "../include/anitomy/format.hpp"
#include "../include/anitomy/json.hpp"
#include "../include/anitomy/print.hpp"
#include "../include/anitomy/version.hpp"

namespace {

using namespace anitomy;
using namespace anitomy::detail;  // don't try this at home

void print_usage() {
  std::println("anitomy {}", version());
  std::println("Usage: anitomy [options...] <input>");
  std::println("Help: anitomy --help");
}

void print_help() {
  std::println("anitomy {}", version());
  std::println("Usage: anitomy [options...] <input>");
  std::println("Options:");
  std::println("  --help             You are here");
  std::println("  --stdin            Use standard input");
  std::println("  --format=<format>  Set output format (`json` or `table`)");
  std::println("  --pretty           Pretty print JSON");
}

void print_error(std::string_view message) {
  std::println(std::cerr, "Error: {}", message);
}

void print_elements_table(const std::vector<Element>& elements) {
  using row_t = std::vector<std::string>;

  std::vector<row_t> rows;
  for (const auto& element : elements) {
    std::string kind{to_string(element.kind)};
    rows.emplace_back(row_t{kind, element.value});
  }

  print_table({"Element", "Value"}, rows);
}

void print_elements_json(const std::vector<Element>& elements, bool pretty) {
  json::Value items{json::Value::object_t{}};
  for (const auto& element : elements) {
    items.as_object().emplace(to_string(element.kind), element.value);
  }

  std::print("{}", json::serialize(items, pretty));
}

bool is_trivial_token(const Token& token) noexcept {
  using enum TokenKind;
  switch (token.kind) {
    case OpenBracket:
    case CloseBracket:
    case Delimiter:
      return true;
    default:
      return false;
  };
}

void print_tokens_table(const std::vector<Token>& tokens, bool verbose) {
  using row_t = std::vector<std::string>;

  std::vector<row_t> rows;
  for (const auto& token : tokens) {
    if (!verbose && is_trivial_token(token)) continue;
    rows.emplace_back(row_t{
        std::string{to_string(token.kind)},
        std::string{token.keyword ? to_string(token.keyword->kind) : ""},
        std::string{token.element_kind ? to_string(*token.element_kind) : ""},
        token.value,
    });
  }

  print_table({"Token", "Keyword", "Element", "Value"}, rows);
}

void print_tokens_json(const std::vector<Token>& tokens, bool pretty, bool verbose) {
  json::Value items{json::Value::array_t{}};
  for (const auto& token : tokens) {
    if (!verbose && is_trivial_token(token)) continue;
    items.as_array().emplace_back(token.value);
  }

  std::print("{}", json::serialize(items, pretty));
}

}  // namespace

int main(int argc, char* argv[]) {
  const CommandLine cli{argc, argv};

  if (cli.contains("help")) {
    print_help();
    return 0;
  }
  if (cli.input().empty()) {
    print_usage();
    return 1;
  }

  const Options options;
  Tokenizer tokenizer{cli.input()};
  tokenizer.tokenize(options);
  Parser parser{tokenizer.tokens()};
  parser.parse(options);

  const std::string format = cli.get("format", "table");
  if (format != "json" && format != "table") {
    print_error("Invalid format value");
    return 1;
  }

  const bool debug = cli.contains("debug");
  const bool pretty = cli.contains("pretty");
  const bool verbose = cli.contains("verbose");

  if (format == "json") {
    if (debug) {
      print_tokens_json(parser.tokens(), pretty, verbose);
    } else {
      print_elements_json(parser.elements(), pretty);
    }
  } else if (format == "table") {
    if (debug) {
      print_tokens_table(parser.tokens(), verbose);
    } else {
      print_elements_table(parser.elements());
    }
  }

  return 0;
}
