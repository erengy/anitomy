# Anitomy

*Anitomy* is a C++ library for parsing anime video filenames. It's accurate, fast, and simple to use.

## Examples

The following filename...

    [TaigaSubs]_Toradora!_(2008)_-_01v2_-_Tiger_and_Dragon_[1280x720_H.264_FLAC][1234ABCD].mkv

...would be resolved into these elements:

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

  auto& elements = anitomy.elements();

  // Elements are iterable, where each element is a category-value pair
  for (auto& element : elements) {
    std::wcout << element.first << L"\t" << element.second << std::endl;
  }
  std::wcout << std::endl;

  // You can access values directly by using get() and get_all() methods
  std::wcout << elements.get(anitomy::kElementAnimeTitle) << L" #" <<
                elements.get(anitomy::kElementEpisodeNumber) << L" by " <<
                elements.get(anitomy::kElementReleaseGroup) << std::endl;

  return 0;
}
```

...which will output:

```
10      mkv
11      [Ouroboros]_Fullmetal_Alchemist_Brotherhood_-_01
6       01
1       Fullmetal Alchemist Brotherhood
14      Ouroboros

Fullmetal Alchemist Brotherhood #01 by Ouroboros
```

## License

*Anitomy* is licensed under [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.html).