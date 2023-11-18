#pragma once

#include <format>
#include <string>
#include <string_view>

#define ANITOMY_VERSION_MAJOR 2
#define ANITOMY_VERSION_MINOR 0
#define ANITOMY_VERSION_PATCH 0
#define ANITOMY_VERSION_PRE "alpha"

namespace anitomy {

[[nodiscard]] inline std::string version() noexcept {
  static std::string version =
      std::format("{}.{}.{}", ANITOMY_VERSION_MAJOR, ANITOMY_VERSION_MINOR, ANITOMY_VERSION_PATCH);

  if (!std::string_view{ANITOMY_VERSION_PRE}.empty()) {
    version += std::format("-{}", ANITOMY_VERSION_PRE);
  }

  return version;
}

}  // namespace anitomy
