#pragma once

#include <string_view>
#include <vector>

#include <anitomy/detail/parser.hpp>
#include <anitomy/detail/tokenizer.hpp>
#include <anitomy/element.hpp>
#include <anitomy/options.hpp>

namespace anitomy {

inline std::vector<Element> parse(std::string_view input, Options options = {}) noexcept {
  detail::Tokenizer tokenizer{input};
  tokenizer.tokenize(options);

  detail::Parser parser{tokenizer.tokens()};
  parser.parse(options);

  return parser.elements();
}

}  // namespace anitomy
