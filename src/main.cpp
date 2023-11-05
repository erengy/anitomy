#include <print>
#include <string>
#include <vector>

#include "../include/anitomy.hpp"
#include "../include/anitomy/cli.hpp"
#include "../include/anitomy/format.hpp"
#include "../include/anitomy/json.hpp"
#include "../include/anitomy/print.hpp"

namespace {

void print_usage() {
  std::println("Usage: anitomy [options...] <input>");
  std::println("Help: anitomy --help");
}

void print_help() {
  std::println("Usage: anitomy [options...] <input>");
  std::println("Options:");
  std::println("  --help             You are here");
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
  std::vector<std::pair<std::string, std::string>> items;

  for (const auto& element : elements) {
    items.emplace_back(anitomy::detail::to_string(element.kind), element.value);
  }

  std::print("{}", anitomy::detail::json::serialize(items, pretty));
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

  const std::string format = cli.get("format", "table");
  const bool pretty = cli.contains("pretty");

  if (format == "json") {
    print_elements_json(parser.elements(), pretty);
  } else {
    print_elements(parser.elements());
  }

  return 0;
}
