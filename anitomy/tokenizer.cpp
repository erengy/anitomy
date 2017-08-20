/*
** Copyright (c) 2014-2017, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <iterator>
#include <set>

#include "keyword.h"
#include "string.h"
#include "tokenizer.h"

namespace anitomy {

Tokenizer::Tokenizer(const string_t& filename, Elements& elements,
                     const Options& options, token_container_t& tokens)
    : elements_(elements),
      filename_(filename),
      options_(options),
      tokens_(tokens) {
}

bool Tokenizer::Tokenize() {
  tokens_.reserve(32);  // Usually there are no more than 20 tokens

  TokenizeByBrackets();

  return !tokens_.empty();
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::AddToken(TokenCategory category, bool enclosed,
                         const TokenRange& range) {
  tokens_.push_back(Token(category,
                          filename_.substr(range.offset, range.size),
                          enclosed));
}

void Tokenizer::TokenizeByBrackets() {
  static const std::vector<std::pair<char_t, char_t>> brackets{
      {L'(', L')'},  // U+0028-U+0029 Parenthesis
      {L'[', L']'},  // U+005B-U+005D Square bracket
      {L'{', L'}'},  // U+007B-U+007D Curly bracket
      {L'\u300C', L'\u300D'},  // Corner bracket
      {L'\u300E', L'\u300F'},  // White corner bracket
      {L'\u3010', L'\u3011'},  // Black lenticular bracket
      {L'\uFF08', L'\uFF09'},  // Fullwidth parenthesis
  };

  bool is_bracket_open = false;
  char_t matching_bracket = L'\0';

  auto char_begin = filename_.begin();
  const auto char_end = filename_.end();

  // This is basically std::find_first_of() customized to our needs
  auto find_first_bracket = [&]() -> string_t::const_iterator {
    for (auto it = char_begin; it != char_end; ++it) {
      for (const auto& bracket_pair : brackets) {
        if (*it == bracket_pair.first) {
          matching_bracket = bracket_pair.second;
          return it;
        }
      }
    }
    return char_end;
  };

  auto current_char = char_begin;

  while (current_char != char_end && char_begin != char_end) {
    if (!is_bracket_open) {
      current_char = find_first_bracket();
    } else {
      // Looking for the matching bracket allows us to better handle some rare
      // cases with nested brackets.
      current_char = std::find(char_begin, char_end, matching_bracket);
    }

    const TokenRange range{
        static_cast<size_t>(std::distance(filename_.begin(), char_begin)),
        static_cast<size_t>(std::distance(char_begin, current_char))
    };

    if (range.size > 0)  // Found unknown token
      TokenizeByPreidentified(is_bracket_open, range);

    if (current_char != char_end) {  // Found bracket
      AddToken(kBracket, true, TokenRange{range.offset + range.size, 1});
      is_bracket_open = !is_bracket_open;
      char_begin = ++current_char;
    }
  }
}

void Tokenizer::TokenizeByPreidentified(bool enclosed, const TokenRange& range) {
  std::vector<TokenRange> preidentified_tokens;
  keyword_manager.Peek(filename_, range, elements_, preidentified_tokens);

  size_t offset = range.offset;
  TokenRange subrange{range.offset, 0};

  while (offset < range.offset + range.size) {
    for (const auto& preidentified_token : preidentified_tokens) {
      if (offset == preidentified_token.offset) {
        if (subrange.size > 0)
          TokenizeByDelimiters(enclosed, subrange);
        AddToken(kIdentifier, enclosed, preidentified_token);
        subrange.offset = preidentified_token.offset + preidentified_token.size;
        offset = subrange.offset - 1;  // It's going to be incremented below
        break;
      }
    }
    subrange.size = ++offset - subrange.offset;
  }

  // Either there was no preidentified token range, or we're now about to
  // process the tail of our current range.
  if (subrange.size > 0)
    TokenizeByDelimiters(enclosed, subrange);
}

void Tokenizer::TokenizeByDelimiters(bool enclosed, const TokenRange& range) {
  const string_t delimiters = GetDelimiters(range);

  if (delimiters.empty()) {
    AddToken(kUnknown, enclosed, range);
    return;
  }

  auto char_begin = filename_.begin() + range.offset;
  const auto char_end = char_begin + range.size;
  auto current_char = char_begin;

  while (current_char != char_end) {
    current_char = std::find_first_of(current_char, char_end,
                                      delimiters.begin(), delimiters.end());

    const TokenRange subrange{
        static_cast<size_t>(std::distance(filename_.begin(), char_begin)),
        static_cast<size_t>(std::distance(char_begin, current_char))
    };

    if (subrange.size > 0)  // Found unknown token
      AddToken(kUnknown, enclosed, subrange);

    if (current_char != char_end) {  // Found delimiter
      AddToken(kDelimiter, enclosed,
               TokenRange{subrange.offset + subrange.size, 1});
      char_begin = ++current_char;
    }
  }

  ValidateDelimiterTokens();
}

////////////////////////////////////////////////////////////////////////////////

string_t Tokenizer::GetDelimiters(const TokenRange& range) const {
  string_t delimiters;

  auto is_delimiter = [&](const char_t& c) {
    if (!IsAlphanumericChar(c))
      if (options_.allowed_delimiters.find(c) != string_t::npos)
        if (delimiters.find(c) == string_t::npos)
          return true;
    return false;
  };

  std::copy_if(filename_.begin() + range.offset,
               filename_.begin() + range.offset + range.size,
               std::back_inserter(delimiters), is_delimiter);

  return delimiters;
}

void Tokenizer::ValidateDelimiterTokens() {
  auto is_delimiter_token = [&](token_iterator_t it) {
    return it != tokens_.end() && it->category == kDelimiter;
  };
  auto is_unknown_token = [&](token_iterator_t it) {
    return it != tokens_.end() && it->category == kUnknown;
  };
  auto is_single_character_token = [&](token_iterator_t it) {
    return is_unknown_token(it) && it->content.size() == 1 &&
           it->content.front() != L'-';
  };
  auto append_token_to = [](token_iterator_t token,
                            token_iterator_t append_to) {
    append_to->content.append(token->content);
    token->category = kInvalid;
  };

  for (auto token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->category != kDelimiter)
      continue;
    auto delimiter = token->content.front();
    auto prev_token = FindPreviousToken(tokens_, token, kFlagValid);
    auto next_token = FindNextToken(tokens_, token, kFlagValid);

    // Check for single-character tokens to prevent splitting group names,
    // keywords, episode number, etc.
    if (delimiter != L' ' && delimiter != L'_') {
      if (is_single_character_token(prev_token)) {
        append_token_to(token, prev_token);
        while (is_unknown_token(next_token)) {
          append_token_to(next_token, prev_token);
          next_token = FindNextToken(tokens_, next_token, kFlagValid);
          if (is_delimiter_token(next_token) &&
              next_token->content.front() == delimiter) {
            append_token_to(next_token, prev_token);
            next_token = FindNextToken(tokens_, next_token, kFlagValid);
          }
        }
        continue;
      }
      if (is_single_character_token(next_token)) {
        append_token_to(token, prev_token);
        append_token_to(next_token, prev_token);
        continue;
      }
    }

    // Check for adjacent delimiters
    if (is_unknown_token(prev_token) && is_delimiter_token(next_token)) {
      auto next_delimiter = next_token->content.front();
      if (delimiter != next_delimiter && delimiter != ',') {
        if (next_delimiter == ' ' || next_delimiter == '_') {
          append_token_to(token, prev_token);
        }
      }
    } else if (is_delimiter_token(prev_token) &&
               is_delimiter_token(next_token)) {
      const auto prev_delimiter = prev_token->content.front();
      const auto next_delimiter = next_token->content.front();
      if (prev_delimiter == next_delimiter &&
          prev_delimiter != delimiter) {
        token->category = kUnknown;  // e.g. "&" in "_&_"
      }
    }

    // Check for other special cases
    if (delimiter == '&' || delimiter == '+') {
      if (is_unknown_token(prev_token) && is_unknown_token(next_token)) {
        if (IsNumericString(prev_token->content) &&
            IsNumericString(next_token->content)) {
          append_token_to(token, prev_token);
          append_token_to(next_token, prev_token);  // e.g. "01+02"
        }
      }
    }
  }

  auto remove_if_invalid = std::remove_if(tokens_.begin(), tokens_.end(),
      [](const Token& token) -> bool {
        return token.category == kInvalid;
      });
  tokens_.erase(remove_if_invalid, tokens_.end());
}

}  // namespace anitomy
