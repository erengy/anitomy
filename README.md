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
- Video codec: *H.264*
- Audio codec: *FLAC*
- Checksum: *1234ABCD*

Here's an example code snippet...

```cpp
#include <iostream>
#include <anitomy/anitomy.h>

int main() {
  anitomy::Anitomy a;
  a.Parse(L"[Ouroboros]_Fullmetal_Alchemist_Brotherhood_-_01.mkv");

  auto& e = a.elements();
  std::wcout << e[anitomy::kElementAnimeTitle] << L" #" <<
                e[anitomy::kElementEpisodeNumber] << L" by " <<
                e[anitomy::kElementReleaseGroup] << std::endl;

  return 0;
}
```

...which will output:

    Fullmetal Alchemist Brotherhood #01 by Ouroboros

## License

*Anitomy* is licensed under [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.html).