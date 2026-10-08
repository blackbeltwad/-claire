#include "lexer.h"
#include <cctype>
#include <iostream>
#include <stack>
#include <vector>

#define GUARD(expr)                                                            \
  if (!(expr))                                                                 \
  return false

bool complete_token(std::string *current, std::vector<token> *tokens);
bool valid_id_int_string(std::string *current, std::vector<token> *tokens);

static bool is_within_string = false;

bool tokenize(std::string *string_source) {
  std::string source = *string_source;
  std::string buffer;
  char disjoint_token;
  std::vector<token> tokens;
  std::stack<char> delimiter_tracker;

  for (std::size_t i = 0; i < source.size(); i++) {
    char current = source[i];

    if (is_within_string) {
      if (current == '\'') {
        if (!buffer.empty()) {
          GUARD(complete_token(&buffer, &tokens));
        }

        std::string char_string(1, current);
        GUARD(complete_token(&char_string, &tokens));
      } else {
        buffer += current;
      }

      continue;
    }

    if (is_whitespace(current)) {
      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
    }

    else if (is_identifier_character(current)) {
      buffer += current;
    }

    else if (is_opening_bracket(current)) {
      disjoint_token = current;

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }

      delimiter_tracker.push(disjoint_token);

      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&char_string, &tokens));
    }

    else if (is_closing_bracket(current)) {
      disjoint_token = current;

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }

      if (delimiter_tracker.empty() || delimiter_tracker.top() != '{') {
        std::cerr << "Error: Mismatched closing bracket\n";
        return false;
      }

      delimiter_tracker.pop();

      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&char_string, &tokens));
    }

    else if (is_opening_parenthesis(current)) {
      disjoint_token = current;

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }

      delimiter_tracker.push(disjoint_token);

      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&char_string, &tokens));
    }

    else if (is_closing_parenthesis(current)) {
      disjoint_token = current;

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }

      if (delimiter_tracker.empty() || delimiter_tracker.top() != '(') {
        std::cerr << "Error: Mismatched closing parenthesis\n";
        return false;
      }

      delimiter_tracker.pop();

      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&char_string, &tokens));
    }

    else {
      // Operators such as + - / * =
      disjoint_token = current;

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }

      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&char_string, &tokens));
    }
  }

  if (!buffer.empty()) {
    GUARD(complete_token(&buffer, &tokens));
  }

  if (!delimiter_tracker.empty()) {
    std::cerr << "Error: Missing delimiter pair\n";
    return false;
  }

  if (is_within_string) {
    std::cerr << "Error: Missing String Or Char Pair\n";
    return false;
  }

  for (token i : tokens)
    std::cout << i << ' ';

  return true;
}

bool complete_token(std::string *current, std::vector<token> *tokens) {
  if (current->empty())
    return true;

  if (keyvalues.contains(*current)) {
    tokens->push_back(token{keyvalues[*current], *current});

    if (keyvalues[*current] == token_type::SINGLE_QUOTE) {
      is_within_string = !is_within_string;
    }
  } else {
    GUARD(valid_id_int_string(current, tokens));
  }

  *current = "";
  return true;
}

bool valid_id_int_string(std::string *current, std::vector<token> *tokens) {
  std::string source = *current;

  if (source.empty())
    return true;

  if (is_within_string) {
    tokens->push_back(token{token_type::STRING, source});
    return true;
  }

  bool is_identifier_string = false;
  bool is_integer_literal = false;

  if (is_identifier_start(source[0])) {
    is_identifier_string = true;
  }

  else if (is_digit(source[0])) {
    is_integer_literal = true;
  }

  else {
    std::cerr << "Error: Invalid Identifier Value\n";
    return false;
  }

  for (std::size_t i = 0; i < source.size(); i++) {
    if (is_integer_literal && !is_digit(source[i])) {
      std::cerr << "Error: Invalid Integer Value\n";
      return false;
    }

    if (is_identifier_string && !is_identifier_character(source[i])) {
      std::cerr << "Error: Invalid Identifier Value\n";
      return false;
    }
  }

  if (is_integer_literal) {
    tokens->push_back(token{token_type::INTEGER, source});
  }

  if (is_identifier_string) {
    tokens->push_back(token{token_type::IDENTIFIER, source});
  }

  return true;
}
