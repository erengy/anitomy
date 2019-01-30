/*
** Copyright (c) 2014-2019, Eren Okka
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <iterator>
#include <set>

#include <anitomy/keyword.hpp>
#include <anitomy/string.hpp>
#include <anitomy/tokenizer.hpp>
#include <anitomy/util.hpp>

namespace anitomy {

Tokenizer::Tokenizer(const Options& options, token_container_t& tokens)
    : options_(options),
      tokens_(tokens) {
}

bool Tokenizer::Tokenize(const string_view_t filename) {
  tokens_.reserve(32);  // Usually there are no more than 20 tokens

  TokenizeByBrackets(filename);
  ValidateDelimiterTokens();

  return !tokens_.empty();
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::TokenizeByBrackets(string_view_t view) {
  static const string_t brackets_left =
      L"("        // Parenthesis
      L"["        // Square bracket
      L"{"        // Curly bracket
      L"\u300C"   // Corner bracket
      L"\u300E"   // White corner bracket
      L"\u3010"   // Black lenticular bracket
      L"\uFF08";  // Fullwidth parenthesis
  static const string_t brackets_right =
      L")]}\u300D\u300F\u3011\uFF09";

  bool is_bracket_open = false;
  char_t matching_bracket{};

  while (!view.empty()) {
    // Looking for the matching bracket allows us to better handle some rare
    // cases with nested brackets.
    const auto pos = !is_bracket_open ? view.find_first_of(brackets_left) :
                                        view.find(matching_bracket);

    if (pos > 0) {
      TokenizeByDelimiters(is_bracket_open, view.substr(0, pos));
    }
    if (pos == view.npos) {
      return;
    }

    if (!is_bracket_open) {
      const auto bracket_index = brackets_left.find(view.at(pos));
      matching_bracket = brackets_right.at(bracket_index);
    }
    is_bracket_open = !is_bracket_open;

    tokens_.push_back(
        Token{TokenType::Bracket, string_t{view.substr(pos, 1)}, true});
    view.remove_prefix(pos + 1);
  }
}

void Tokenizer::TokenizeByDelimiters(bool enclosed, string_view_t view) {
  while (!view.empty()) {
    const auto pos = view.find_first_of(options_.allowed_delimiters);
    if (pos > 0) {
      tokens_.push_back(
          Token{TokenType::Unknown, string_t{view.substr(0, pos)}, enclosed});
    }
    if (pos == view.npos) {
      return;
    }
    tokens_.push_back(
        Token{TokenType::Delimiter, string_t{view.substr(pos, 1)}, enclosed});
    view.remove_prefix(pos + 1);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::ValidateDelimiterTokens() {
  auto is_delimiter_token = [&](token_iterator_t it) {
    return it != tokens_.end() && it->type == TokenType::Delimiter;
  };
  auto is_unknown_token = [&](token_iterator_t it) {
    return it != tokens_.end() && it->type == TokenType::Unknown;
  };
  auto is_single_character_token = [&](token_iterator_t it) {
    return is_unknown_token(it) && it->content.size() == 1 &&
           it->content.front() != L'-';
  };
  auto append_token_to = [](token_iterator_t token,
                            token_iterator_t append_to) {
    append_to->content.append(token->content);
    token->type = TokenType::Invalid;
  };

  for (auto token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->type != TokenType::Delimiter)
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
        token->type = TokenType::Unknown;  // e.g. "&" in "_&_"
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
        return token.type == TokenType::Invalid;
      });
  tokens_.erase(remove_if_invalid, tokens_.end());
}

}  // namespace anitomy
