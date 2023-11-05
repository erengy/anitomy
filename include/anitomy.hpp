#pragma once

#include <string_view>
#include <vector>

#include "anitomy/options.hpp"
#include "anitomy/parser.hpp"
#include "anitomy/tokenizer.hpp"

namespace anitomy {

inline std::vector<Element> parse(std::string_view input, Options options = {}) noexcept {
  detail::Tokenizer tokenizer{input};
  tokenizer.tokenize(options);

  detail::Parser parser{tokenizer.tokens()};
  parser.parse(options);

  return parser.elements();
}

}  // namespace anitomy
