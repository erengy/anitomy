# Anitomy

[![](https://img.shields.io/github/license/erengy/anitomy)](https://github.com/erengy/anitomy/blob/master/LICENSE)
[![](https://img.shields.io/github/sponsors/erengy?logo=github)](https://github.com/sponsors/erengy)

Anitomy is a C++ library and a command-line tool for parsing anime video filenames.

> [!IMPORTANT]
> The library is currently being rewritten in the `develop` branch. The documentation below may be out of date.

## Example

    [TaigaSubs]_Toradora!_(2008)_-_01v2_-_Tiger_and_Dragon_[1080p_H.264_FLAC][BAD7A16A].mkv

Element|Value
:------|:----
Release group|`TaigaSubs`
Title|`Toradora!`
Year|`2008`
Episode number|`01`
Release version|`2`
Episode title|`Tiger and Dragon`
Video resolution|`1080p`
Video term|`H.264`
Audio term|`FLAC`
File checksum|`BAD7A16A`
File extension|`mkv`

## Usage

### Library

Requires a compiler that supports [C++23](https://en.cppreference.com/w/cpp/compiler_support). Currently only tested with the latest version of MSVC.

```cpp
#include <print>
#include <anitomy.hpp>

int main() {
  auto elements = anitomy::parse("[Ouroboros] Fullmetal Alchemist Brotherhood - 01");

  for (auto [kind, value] : elements) {
    std::println("{:<16}{}", anitomy::to_string(kind), value);
  }

  return 0;
}
```

```
release_group   Ouroboros
title           Fullmetal Alchemist Brotherhood
episode_number  01
```

### CLI

Use `--help` to see available options.

```bash
anitomy --format=json --pretty "[Ouroboros] Fullmetal Alchemist Brotherhood - 01"
```

```json
{
  "title": "Fullmetal Alchemist Brotherhood",
  "episode_number": "01",
  "release_group": "Ouroboros"
}
```

## FAQ

> **How does it work?**

Anitomy makes some educated guesses on how filenames are formed. In more technical terms, the input is split into a sequence of tokens, and the tokens are inspected for various keywords and patterns to extract the elements.

> **What should I do if a filename is not being parsed correctly?**

Please report the issue, though it may not be possible to resolve a terribly ambiguous naming scheme.

> **Can I use it for other media such as movies or TV series?**

It should work to some extent, but the library is mostly optimized for anime.

> **Can I use it on Unicode filenames?**

Yes, just make sure your input is UTF-8 encoded and preferably in composed form.

> **Can I use it in another programming language?**

See [other repositories](https://github.com/search?q=anitomy&type=repositories) for related projects.

## Migration

*TODO: Explain the differences between v1 and v2.*

- C++17 → C++23
- `anitomy::Anitomy::Parse` → `anitomy::parse`
- UTF-16 `std::wstring` → UTF-8 `std::string`
- Options
- Elements
    - Order by position

## License

Anitomy is licensed under [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/FAQ/).
