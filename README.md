# Anitomy

*Anitomy* is a C++ library for parsing anime video filenames. It's accurate, fast, and simple to use.

## Examples

The following filename...

    [TaigaSubs]_Toradora!_(2008)_-_01v2_-_Tiger_and_Dragon_[1280x720_H.264_FLAC][1234ABCD].mkv

...is resolved into these elements:

- Release group: *TaigaSubs*
- Anime title: *Toradora!*
- Anime year: *2008*
- Episode number: *01*
- Release version: *2*
- Episode title: *Tiger and Dragon*
- Video resolution: *1280x720*
- Video term: *H.264*
- Audio term: *FLAC*
- File checksum: *1234ABCD*

Here's an example code snippet...

```cpp
#include <iostream>
#include <anitomy/anitomy.h>

int main() {
  anitomy::Anitomy anitomy;
  anitomy.Parse(L"[Ouroboros]_Fullmetal_Alchemist_Brotherhood_-_01.mkv");

  const auto& elements = anitomy.elements();

  // Elements are iterable, where each element is a category-value pair
  for (const auto& element : elements) {
    std::wcout << element.first << '\t' << element.second << '\n';
  }
  std::wcout << '\n';

  // You can access values directly by using get() and get_all() methods
  std::wcout << elements.get(anitomy::kElementAnimeTitle) << L" #" <<
                elements.get(anitomy::kElementEpisodeNumber) << L" by " <<
                elements.get(anitomy::kElementReleaseGroup) << '\n';

  return 0;
}
```

...which will output:

```
12      mkv
13      [Ouroboros]_Fullmetal_Alchemist_Brotherhood_-_01
7       01
2       Fullmetal Alchemist Brotherhood
16      Ouroboros

Fullmetal Alchemist Brotherhood #01 by Ouroboros
```

## How does it work?

Suppose that we're working on the following filename:

    "Spice_and_Wolf_Ep01_[1080p,BluRay,x264]_-_THORA.mkv"

The filename is first stripped off of its extension and split into groups. Groups are determined by the position of brackets:

    "Spice_and_Wolf_Ep01_", "1080p,BluRay,x264", "_-_THORA"

Each group is then split into tokens. In our current example, the delimiter for the enclosed group is `,`, while the words in other groups are separated by `_`:

    "Spice", "and", "Wolf", "Ep01", "1080p", "BluRay", "x264", "-", "THORA"

Note that brackets and delimiters are actually stored as tokens. Here, identified tokens are omitted for our convenience.

Once the tokenizer is done, the parser comes into effect. First, all tokens are compared against a set of known patterns and keywords. This process generally leaves us with nothing but the release group, anime title, episode number and episode title:

    "Spice", "and", "Wolf", "Ep01", "-"

The next step is to look for the episode number. Each token that contains a number is analyzed. Here, `Ep01` is identified because it begins with a known episode prefix:

    "Spice", "and", "Wolf", "-"

Finally, remaining tokens are combined to form the anime title, which is `Spice and Wolf`. The complete list of elements identified by *Anitomy* is as follows:

- Anime title: *Spice and Wolf*
- Episode number: *01*
- Video resolution: *1080p*
- Source: *BluRay*
- Video term: *x264*
- Release group: *THORA*

## Why should I use it?

Anime video files are commonly named in a format where the anime title is followed by the episode number, and all the technical details are enclosed within brackets. However, fansub groups tend to use their own naming conventions, and the problem is more complicated than it first appears:

- Element order is not always the same.
- Technical information is not guaranteed to be enclosed.
- Brackets and parentheses may be grouping symbols or a part of the anime/episode title.
- Space and underscore are not the only delimiters in use.
- A single filename may contain multiple delimiters.

There are so many cases to cover that it's simply not possible to parse all filenames solely with regular expressions. *Anitomy* tries a different approach, and it succeeds: It's able to parse tens of thousands of filenames per second, with great accuracy.

The following projects make use of *Anitomy*:

- [Taiga](https://github.com/erengy/taiga)
- [MAL Updater OS X](https://github.com/chikorita157/malupdaterosx-cocoa)
- [Hachidori](https://github.com/chikorita157/hachidori)
- [Shinjiru](https://github.com/Kazakuri/Shinjiru)

See [other repositories](https://github.com/search?utf8=%E2%9C%93&q=anitomy) for related projects (e.g. interfaces, ports, wrappers).

## Are there any exceptions?

Yes, unfortunately. *Anitomy* fails to identify the anime title and episode number on rare occasions, mostly due to bad naming conventions. See the examples below.

    Arigatou.Shuffle!.Ep08.[x264.AAC][D6E43829].mkv

Here, *Anitomy* would report that this file is the 8th episode of `Arigatou Shuffle!`, where `Arigatou` is actually the name of the fansub group.

    Spice and Wolf 2

Is this the 2nd episode of `Spice and Wolf`, or a batch release of `Spice and Wolf 2`? Without a file extension, there's no way to know. It's up to you consider both cases.

## Suggestions to fansub groups

Please consider abiding by these simple rules before deciding on your naming convention:

- Don't enclose anime title, episode number and episode title within brackets. Enclose everything else, including the name of your group.
- Don't use parentheses to enclose release information; use square brackets instead. Parentheses should only be used if they are a part of the anime/episode title.
- Don't use multiple delimiters in a single filename. If possible, stick with either space or underscore.
- Use a separator (e.g. a dash) between anime title and episode number. There are anime titles that end with a number, which creates ambiguity.
- Indicate the episode interval in batch releases.

## License

*Anitomy* is licensed under [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/FAQ/).
