#include <print>
#include <string>
#include <string_view>
#include <vector>

#include <anitomy.hpp>
#include <anitomy/detail/cli.hpp>
#include <anitomy/detail/cli/table.hpp>
#include <anitomy/detail/cli/util.hpp>
#include <anitomy/detail/json.hpp>
#include <anitomy/version.hpp>
#include <anitomy/detail/cli/args.hpp>

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

void print_json(const std::vector<Element>& elements, bool pretty) {
  std::print("{}", json::serialize(to_json(elements), pretty));
}

void print_json(const std::vector<Token>& tokens, bool pretty, bool verbose) {
  std::print("{}", json::serialize(to_json(tokens, verbose), pretty));
}

void print_table(const std::vector<Element>& elements) {
  detail::print_table({"Element", "Value"}, to_rows(elements));
}

void print_table(const std::vector<Token>& tokens, bool verbose) {
  detail::print_table({"Token", "Keyword", "Element", "Value"}, to_rows(tokens, verbose));
}

}  // namespace

int main(int argc, char* argv[]) {
  const auto args = make_utf8_args(argc, argv);
  const CommandLine cli{make_arg_views(args)};

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
      print_json(parser.tokens(), pretty, verbose);
    } else {
      print_json(parser.elements(), pretty);
    }
  } else if (format == "table") {
    if (debug) {
      print_table(parser.tokens(), verbose);
    } else {
      print_table(parser.elements());
    }
  }

  return 0;
}
