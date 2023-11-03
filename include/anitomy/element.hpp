#pragma once

#include <string>

namespace anitomy {

enum class ElementKind {
  AnimeSeason,
  AnimeTitle,
  AnimeType,
  AnimeYear,
  AudioTerm,
  DeviceCompatibility,
  EpisodeNumber,
  EpisodeTitle,
  FileChecksum,
  FileExtension,
  Language,
  Other,
  ReleaseGroup,
  ReleaseInformation,
  ReleaseVersion,
  Source,
  Subtitles,
  VideoResolution,
  VideoTerm,
  VolumeNumber,
};

struct Element {
  ElementKind kind;
  std::string value;
};

}  // namespace anitomy
