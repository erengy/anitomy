#pragma once

#include <string>

namespace anitomy {

enum class ElementKind {
  AudioTerm,
  DeviceCompatibility,
  Episode,
  EpisodeTitle,
  FileChecksum,
  FileExtension,
  Language,
  Other,
  ReleaseGroup,
  ReleaseInformation,
  ReleaseVersion,
  Season,
  Source,
  Subtitles,
  Title,
  Type,
  VideoResolution,
  VideoTerm,
  Volume,
  Year,
};

struct Element {
  ElementKind kind;
  std::string value;
  size_t position;
};

}  // namespace anitomy
