#pragma once

namespace anitomy {

struct Options {
  bool parse_episode_number = true;
  bool parse_episode_title = true;
  bool parse_file_checksum = true;
  bool parse_file_extension = true;
  bool parse_release_group = true;
  bool parse_season = true;
  bool parse_title = true;
  bool parse_video_resolution = true;
  bool parse_year = true;
};

}  // namespace anitomy
