#pragma once

namespace anitomy {

struct Options {
  bool parse_anime_season = true;
  bool parse_anime_year = true;
  bool parse_episode_number = true;
  bool parse_episode_title = true;
  bool parse_file_checksum = true;
  bool parse_file_extension = true;
  bool parse_release_group = true;
  bool parse_video_resolution = true;
};

}  // namespace anitomy
