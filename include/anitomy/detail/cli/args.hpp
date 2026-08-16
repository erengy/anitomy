#pragma once

#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

namespace anitomy::detail {

using utf8_args_t = std::vector<std::string>;
using arg_views_t = std::vector<std::string_view>;

#ifdef _WIN32

inline std::string wide_to_utf8(std::wstring_view input) {
  if (input.empty()) return {};

  const int size = WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      input.data(),
      static_cast<int>(input.size()),
      nullptr,
      0,
      nullptr,
      nullptr);

  if (size <= 0) return {};

  std::string output(static_cast<size_t>(size), '\0');

  WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      input.data(),
      static_cast<int>(input.size()),
      output.data(),
      size,
      nullptr,
      nullptr);

  return output;
}

inline utf8_args_t make_utf8_args(int argc, char* argv[]) {
  int wide_argc = 0;
  wchar_t** wide_argv = CommandLineToArgvW(GetCommandLineW(), &wide_argc);

  if (wide_argv == nullptr) {
    utf8_args_t args;
    args.reserve(static_cast<size_t>(argc));

    for (int i = 0; i < argc; ++i) {
      args.emplace_back(argv[i]);
    }

    return args;
  }

  utf8_args_t args;
  args.reserve(static_cast<size_t>(wide_argc));

  for (int i = 0; i < wide_argc; ++i) {
    args.push_back(wide_to_utf8(wide_argv[i]));
  }

  LocalFree(wide_argv);
  return args;
}

#else

inline utf8_args_t make_utf8_args(int argc, char* argv[]) {
  utf8_args_t args;
  args.reserve(static_cast<size_t>(argc));

  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  return args;
}

#endif

inline arg_views_t make_arg_views(const utf8_args_t& args) {
  arg_views_t views;
  views.reserve(args.size());

  for (const auto& arg : args) {
    views.emplace_back(arg);
  }

  return views;
}

}  // namespace anitomy::detail