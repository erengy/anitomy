# Anitomy

[![](https://img.shields.io/github/license/erengy/anitomy)](https://github.com/erengy/anitomy/blob/master/LICENSE)
[![](https://img.shields.io/github/sponsors/erengy?logo=github)](https://github.com/sponsors/erengy)

Anitomy is a C++ library and a command-line tool for parsing anime video filenames.

> [!IMPORTANT]
> The library is currently being rewritten in the `develop` branch. The documentation below may be out of date.

## Example

    [TaigaSubs]_Toradora!_(2008)_-_01v2_-_Tiger_and_Dragon_[1280x720_H.264_FLAC][1234ABCD].mkv

Element|Value
:------|:----
Release group|`TaigaSubs`
Anime title|`Toradora!`
Anime year|`2008`
Episode number|`01`
Release version|`2`
Episode title|`Tiger and Dragon`
Video resolution|`1280x720`
Video term|`H.264`
Audio term|`FLAC`
File checksum|`1234ABCD`
File extension|`mkv`

## Usage

### Library

Requires a compiler that supports [C++23](https://en.cppreference.com/w/cpp/compiler_support). Currently only tested with the latest version of MSVC.

```cpp
#include <print>
#include <anitomy.hpp>

int main() {
  auto elements = anitomy::parse("[Ouroboros] Fullmetal Alchemist Brotherhood - 01.mkv");

  for (auto [kind, value] : elements) {
    std::println("{}\t{}", anitomy::to_string(kind), value);
  }

  return 0;
}
```

```
episode_number  01
anime_title     Fullmetal Alchemist Brotherhood
release_group   Ouroboros
```

### CLI

Use `--help` to see available options.

```bash
anitomy --format=json --pretty "[Ouroboros] Fullmetal Alchemist Brotherhood - 01.mkv"
```

```json
{
  "anime_title": "Fullmetal Alchemist Brotherhood",
  "episode_number": "01",
  "release_group": "Ouroboros"
}
```

## How does it work?

Suppose that we're working on the following filename:

> `Spice_and_Wolf_Ep01_[1080p,BluRay,x264]_-_THORA.mkv`

The filename is first split into tokens (brackets and delimiters are omitted here for our convenience):

> `Spice` `and` `Wolf` `Ep01` `1080p` `BluRay` `x264` `THORA` `mkv`

All tokens are compared against a set of known patterns and keywords. This process generally leaves us with nothing but the release group, anime title, episode number and episode title:

> `Spice` `and` `Wolf` `Ep01`

The next step is to look for the episode number. Each token that contains a number is analyzed. Here, `Ep01` is identified because it begins with a known episode prefix:

> `Spice` `and` `Wolf`

Finally, remaining tokens are combined to form the anime title, which is `Spice and Wolf`. The complete list of elements identified by Anitomy is as follows:

Element|Value
:------|:----
Anime title|`Spice and Wolf`
Episode number|`01`
Video resolution|`1080p`
Source|`BluRay`
Video term|`x264`
Release group|`THORA`
File extension|`mkv`

## Why should I use it?

Anime video files are commonly named in a format where the anime title is followed by the episode number, and all the technical details are enclosed in brackets. However, fansub groups tend to use their own naming conventions, and the problem is more complicated than it first appears:

- Element order is not always the same.
- Technical information is not guaranteed to be enclosed.
- Brackets and parentheses may be grouping symbols or a part of the anime/episode title.
- Space and underscore are not the only delimiters in use.
- A single filename may contain multiple delimiters.

There are so many cases to cover that it's simply not possible to parse all filenames solely with regular expressions. Anitomy tries a different approach, and it succeeds: It's able to parse tens of thousands of filenames per second, with great accuracy.

The following projects make use of Anitomy:

- [Taiga](https://github.com/erengy/taiga)
- [MAL Updater OS X](https://github.com/chikorita157/malupdaterosx-cocoa)
- [Hachidori](https://github.com/chikorita157/hachidori)
- [Shinjiru](https://github.com/Kazakuri/Shinjiru)

See [other repositories](https://github.com/search?utf8=%E2%9C%93&q=anitomy) for related projects (e.g. interfaces, ports, wrappers).

## Are there any exceptions?

Yes, unfortunately. Anitomy fails to identify the anime title and episode number on rare occasions, mostly due to bad naming conventions. See the examples below.

    Arigatou.Shuffle!.Ep08.[x264.AAC][D6E43829].mkv

Here, Anitomy would report that this file is the 8th episode of `Arigatou Shuffle!`, where `Arigatou` is actually the name of the fansub group.

    Spice and Wolf 2

Is this the 2nd episode of `Spice and Wolf`, or a batch release of `Spice and Wolf 2`? Without a file extension, there's no way to know. It's up to you to consider both cases.

## Suggestions to fansub groups

Please consider abiding by these simple rules before deciding on your naming convention:

- Don't enclose anime title, episode number and episode title in brackets. Enclose everything else, including the name of your group.
- Don't use parentheses to enclose release information; use square brackets instead. Parentheses should only be used if they are a part of the anime/episode title.
- Don't use multiple delimiters in a single filename. If possible, stick with either space or underscore.
- Use a separator (e.g. a dash) between anime title and episode number. There are anime titles that end with a number, which creates ambiguity.
- Indicate the episode interval in batch releases.

## License

Anitomy is licensed under [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/FAQ/).
