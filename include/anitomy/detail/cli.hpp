#pragma once

#include <iostream>
#include <map>
#include <ranges>
#include <regex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace anitomy::detail {

class CommandLine final {
public:
  using args_t = std::vector<std::string_view>;

  explicit CommandLine(int argc, char* argv[]) noexcept : CommandLine{parse_args(argc, argv)} {
  }

  explicit CommandLine(args_t args) noexcept : args_{args} {
    process_args(args_);
  }

  [[nodiscard]] constexpr std::string_view input() const noexcept {
    return input_;
  }

  [[nodiscard]] inline bool contains(std::string_view option) const noexcept {
    return options_.contains(option);
  }

  [[nodiscard]] inline std::string get(std::string_view option,
                                       std::string default_value = {}) const noexcept {
    const auto it = options_.find(option);
    return it != options_.end() ? it->second : default_value;
  }

private:
  [[nodiscard]] static inline args_t parse_args(int argc, char* argv[]) noexcept {
    return std::span{argv, static_cast<size_t>(argc)} | std::ranges::to<args_t>();
  }

  inline void process_args(std::span<std::string_view> args) noexcept {
    if (!args.empty()) {
      args = args | std::views::drop(1);
    }

    if (!args.empty()) {
      if (!args.back().starts_with("-")) {
        input_ = unquote(args.back());
        args = args | std::views::take(args.size() - 1);
      }
    }

    for (auto arg : args) {
      auto [option, value] = parse_option(std::string{arg});
      if (!option.empty()) options_[option] = value;
    }

    if (options_.contains("stdin")) {
      std::getline(std::cin, input_);
    }
  }

  [[nodiscard]] static inline std::pair<std::string, std::string> parse_option(
      const std::string& arg) noexcept {
    static const std::regex pattern{"--([a-z-]+)(?:=([^ ]*))?"};
    std::smatch matches;
    if (!std::regex_match(arg, matches, pattern)) return {};
    return std::make_pair(matches[1].str(), matches[2].str());
  }

  [[nodiscard]] static constexpr std::string_view unquote(std::string_view view) noexcept {
    if (view.starts_with('"')) view.remove_prefix(1);
    if (view.ends_with('"')) view.remove_suffix(1);
    return view;
  }

  args_t args_;
  std::string input_;
  std::map<std::string, std::string, std::less<>> options_;
};

}  // namespace anitomy::detail
