#pragma once

#include <string>

namespace anitomy {

enum class ElementKind {
  AudioTerm,
  Device,
  Episode,
  EpisodeTitle,
  FileChecksum,
  FileExtension,
  Language,
  Other,
  Part,
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
