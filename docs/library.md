# Library

## `anitomy::parse` function

### Parameters

- `std::string_view input` - Filename to be parsed. Must be UTF-8 encoded and should be in composed form (NFC/NFKC).

- `anitomy::Options options` - Set any option to `false` in order to disable the corresponding parser. By default, all parsers are enabled.

### Output

Returns parsed elements as `std::vector<anitomy::Element>`, ordered according to their position in the input. Note that there may be multiple elements of the same kind.

## `anitomy::Element` struct

- `anitomy::ElementKind kind` - Can be `Title`, `Episode`, etc. Convertable to string using `std::format`.

- `std::string value` - UTF-8 encoded. Most values are as they appear in the input, while `Title` and `EpisodeTitle` are transformed (e.g. underscores are converted to space).

## Error handling

Anitomy does not throw any exceptions.

The only thing you should check for is whether an element exists in the output or not.
