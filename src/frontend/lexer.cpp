#include "lexer.h"
#include <cctype>
#include <iostream>
#include <stack>
#include <vector>

bool complete_token(std::string *current, std::vector<token> *tokens);

bool tokenize(std::string *string_source) {
  std::string source = *string_source;
  std::string buffer;
  char disjoint_token;
  std::vector<token> tokens;
  std::stack<char> delimiter_tracker;

  for (std::size_t i = 0; i < source.size(); i++) {
    if (is_whitespace(source[i])) {
      if (!buffer.empty()) {
        buffer = "";
      }
    } else if (is_digit(source[i])) {

    } else if (is_letter(source[i])) {

    } else if (is_opening_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        complete_token(&buffer, &tokens);
      }
    } else if (is_closing_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        complete_token(&buffer, &tokens);
      }
      delimiter_tracker.pop();
      std::string char_string(1, disjoint_token);
      complete_token(&char_string, &tokens);

    } else if (is_opening_parenthesis(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        complete_token(&buffer, &tokens);
      }
      delimiter_tracker.push(disjoint_token);
      std::string char_string(1, disjoint_token);
      complete_token(&char_string, &tokens);
    } else if (is_closing_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        complete_token(&buffer, &tokens);
      }
      delimiter_tracker.pop();
      std::string char_string(1, disjoint_token);
      complete_token(&char_string, &tokens);
    }

    else {
      // These should be + - / * =
      disjoint_token = source[i];

      if (!buffer.empty()) {
        complete_token(&buffer, &tokens);
      }
      std::string char_string(1, disjoint_token);
      complete_token(&char_string, &tokens);
    }
  }

  if (!delimiter_tracker.empty()) {
    std::cerr << "Error: Missing Closing Pair\n";
    return false;
  }

  return true;
}

bool complete_token(std::string *current, std::vector<token> *tokens) {
  // if my logic is right the else should not hit well maybe idek
  if (keyvalues.contains(*current)) {
    tokens->push_back(token{keyvalues[*current], *current});
  }

  *current = "";
  return true;
}
