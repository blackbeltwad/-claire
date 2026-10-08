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
static bool is_whithin_string = false;

bool tokenize(std::string *string_source) {
  std::string source = *string_source;
  std::string buffer;
  char disjoint_token;
  std::vector<token> tokens;
  std::stack<char> delimiter_tracker;
  for (std::size_t i = 0; i < source.size(); i++) {
    if (is_whitespace(source[i])) {
      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
    } else if (is_identifier_character(source[i])) {
      buffer += source[i];
    } else if (is_opening_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
      delimiter_tracker.push(disjoint_token);
      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&buffer, &tokens));
    }
    //
    else if (is_closing_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
      delimiter_tracker.pop();
      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&buffer, &tokens));
    }
    //
    else if (is_opening_parenthesis(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
      delimiter_tracker.push(disjoint_token);
      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&buffer, &tokens));
    }
    //
    else if (is_closing_bracket(source[i])) {
      disjoint_token = source[i];

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
      delimiter_tracker.pop();
      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&buffer, &tokens));
    } else {
      // These should be + - / * =
      disjoint_token = source[i];

      if (!buffer.empty()) {
        GUARD(complete_token(&buffer, &tokens));
      }
      std::string char_string(1, disjoint_token);
      GUARD(complete_token(&buffer, &tokens));
    }
  }

  if (!delimiter_tracker.empty()) {
    std::cerr << "Error: Missing Parenthesis Pair\n";
    return false;
  }

  if (is_whithin_string) {
    std::cerr << "Error: Missing String Or Char Pair\n";
    return false;
  }

  return true;
}

bool complete_token(std::string *current, std::vector<token> *tokens) {
  // if my logic is right the else should not hit well maybe idek
  if (keyvalues.contains(*current)) {
    tokens->push_back(token{keyvalues[*current], *current});
    if (keyvalues[*current] == token_type::SINGLE_QUOTE) {
      is_whithin_string ? is_whithin_string = false : is_whithin_string = true;
    }
  } else {
    if (!valid_id_int_string(current, tokens)) {
      return false;
    }
  }

  *current = "";
  return true;
}

bool valid_id_int_string(std::string *current, std::vector<token> *tokens) {
  std::string source = *current;
  bool is_identifier_string = false;
  bool is_integer_literal = false;

  if (is_whithin_string) {
    tokens->push_back(token{token_type::STRING, source});
    return true;
  }

  if (is_identifier_start(source[0])) {
    is_identifier_string = true;
  }

  else if (is_digit(source[0])) {
    is_integer_literal = true;
  }

  else {
    std::cerr << "Error: Invalid Identifer Value \n";
    return false;
  }

  for (std::size_t i = 0; i < source.size(); i++) {
    if (is_integer_literal) {
      if (!is_digit(source[i])) {
        std::cerr << "Error: Invalid Identifer Value \n";
        return false;
      }
    }
    if (is_identifier_string) {
      if (!is_identifier_character(source[i])) {
        std::cerr << "Error: Invalid Identifer Value \n";
        return false;
      }
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
