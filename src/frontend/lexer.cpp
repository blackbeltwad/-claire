#include "lexer.h"

#include <cctype>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>

namespace eclaire::lexer {
namespace {

const std::unordered_map<std::string, token_type> keywords = {
    {"fn", token_type::FN},         {"spell", token_type::SPELL},
    {"invoke", token_type::INVOKE}, {"release", token_type::RELEASE},
    {"let", token_type::LET},       {"if", token_type::IF},
    {"elif", token_type::ELIF},     {"else", token_type::ELSE},
    {"while", token_type::WHILE},   {"int", token_type::TYPE},
    {"string", token_type::TYPE}};

bool is_whitespace(char character) {
  return std::isspace(static_cast<unsigned char>(character)) != 0;
}

bool is_digit(char character) {
  return std::isdigit(static_cast<unsigned char>(character)) != 0;
}

bool is_letter(char character) {
  return std::isalpha(static_cast<unsigned char>(character)) != 0;
}

bool is_identifier_start(char character) {
  return is_letter(character) || character == '_';
}

bool is_identifier_character(char character) {
  return is_identifier_start(character) || is_digit(character);
}

bool is_opening_delimiter(char character) {
  return character == '(' || character == '{' || character == '[';
}

bool is_new_line(char character) { return character == '\n'; }

bool is_closing_delimiter(char character) {
  return character == ')' || character == '}' || character == ']';
}

bool delimiters_match(char opening, char closing) {
  return (opening == '(' && closing == ')') ||
         (opening == '{' && closing == '}') ||
         (opening == '[' && closing == ']');
}

bool is_operator(char character) {
  switch (character) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '<':
  case '>':
  case '!':
  case '&':
  case '|':
    return true;
  default:
    return false;
  }
}

bool next_is_equal(const std::string &source, std::size_t i) {
  return i + 1 < source.size() && source[i + 1] == '=';
}

// Prints Error: row:col: so every diagnostic has a position.
std::ostream &error_at(loc where) {
  return std::cerr << "Error: " << where.row << ':' << where.col << ": ";
}

struct open_delimiter {
  char character;
  loc location;
};

// where is the position of the word's first character.
bool emit_word(const std::string &word, loc where, std::vector<token> &tokens) {
  if (word.empty()) {
    return true;
  }

  const auto keyword = keywords.find(word);

  if (keyword != keywords.end()) {
    tokens.push_back({keyword->second, word, where});
    return true;
  }

  if (is_digit(word.front())) {
    for (char character : word) {
      if (!is_digit(character)) {
        error_at(where) << "invalid integer literal: " << word << '\n';
        return false;
      }
    }

    tokens.push_back({token_type::INTEGER, word, where});
    return true;
  }

  if (!is_identifier_start(word.front())) {
    error_at(where) << "invalid token: " << word << '\n';
    return false;
  }

  for (char character : word) {
    if (!is_identifier_character(character)) {
      error_at(where) << "invalid identifier: " << word << '\n';
      return false;
    }
  }

  tokens.push_back({token_type::IDENTIFIER, word, where});
  return true;
}

bool emit_punctuation(char character, loc where, std::vector<token> &tokens) {
  switch (character) {
  case ',':
    tokens.push_back({token_type::COMMA, ",", where});
    return true;
  case '(':
    tokens.push_back({token_type::L_PAREN, "(", where});
    return true;
  case ')':
    tokens.push_back({token_type::R_PAREN, ")", where});
    return true;
  case '{':
    tokens.push_back({token_type::L_BRACE, "{", where});
    return true;
  case '}':
    tokens.push_back({token_type::R_BRACE, "}", where});
    return true;
  case '[':
    tokens.push_back({token_type::L_BRACKET, "[", where});
    return true;
  case ']':
    tokens.push_back({token_type::R_BRACKET, "]", where});
    return true;
  case '=':
    tokens.push_back({token_type::EQUAL, "=", where});
    return true;
  case '\n':
    tokens.push_back({token_type::NEWLINE, " ", where});
    return true;
  default:
    return false;
  }
}

} // namespace

const char *token_type_to_string(token_type type) {
  switch (type) {
  case token_type::FN:
    return "FN";
  case token_type::SPELL:
    return "SPELL";
  case token_type::INVOKE:
    return "INVOKE";
  case token_type::RELEASE:
    return "RELEASE";
  case token_type::LET:
    return "LET";
  case token_type::IDENTIFIER:
    return "IDENTIFIER";
  case token_type::INTEGER:
    return "INTEGER";
  case token_type::STRING:
    return "STRING";
  case token_type::TYPE:
    return "TYPE";
  case token_type::COMMA:
    return "COMMA";
  case token_type::L_PAREN:
    return "L_PAREN";
  case token_type::R_PAREN:
    return "R_PAREN";
  case token_type::L_BRACE:
    return "L_BRACE";
  case token_type::R_BRACE:
    return "R_BRACE";
  case token_type::L_BRACKET:
    return "L_BRACKET";
  case token_type::R_BRACKET:
    return "R_BRACKET";
  case token_type::EQUAL:
    return "EQUAL";
  case token_type::OPERATOR:
    return "OPERATOR";
  case token_type::QUOTE:
    return "QUOTE";
  case token_type::NEWLINE:
    return "NEWLINE";
  case token_type::IF:
    return "IF";
  case token_type::ELIF:
    return "ELIF";
  case token_type::ELSE:
    return "ELSE";
  case token_type::WHILE:
    return "WHILE";
  }

  return "UNKNOWN";
}

std::ostream &operator<<(std::ostream &os, const token &current_token) {
  return os << '{' << token_type_to_string(current_token.type) << ": \""
            << current_token.text << "\" @" << current_token.location.row << ':'
            << current_token.location.col << '}';
}

bool tokenize(const std::string &source, std::vector<token> &tokens) {
  tokens.clear();

  std::string buffer;
  loc buffer_loc; // where the word currently in buffer started
  loc string_loc; // where the current string literal content started
  std::stack<open_delimiter> delimiters;
  bool in_string = false;

  // Position of source[i]. Only step() changes it, so row/col can't drift.
  std::size_t i = 0;
  loc here{1, 1};

  auto step = [&]() {
    if (source[i] == '\n') {
      ++here.row;
      here.col = 1;
    } else {
      ++here.col;
    }
    ++i;
  };

  // Flush the pending word before a delimiter/operator/etc.
  auto flush = [&]() {
    const bool ok = emit_word(buffer, buffer_loc, tokens);
    buffer.clear();
    return ok;
  };

  while (i < source.size()) {
    const char current = source[i];

    if (in_string) {
      if (current == '`') {
        tokens.push_back({token_type::STRING, buffer, string_loc});
        tokens.push_back({token_type::QUOTE, "`", here});
        buffer.clear();
        in_string = false;
      } else {
        buffer += current;
      }

      step();
      continue;
    }

    if (current == '`') {
      if (!flush()) {
        return false;
      }

      tokens.push_back({token_type::QUOTE, "`", here});
      in_string = true;
      string_loc = {here.row, here.col + 1};
    } else if (is_new_line(current)) {
      if (!flush()) {
        return false;
      }

      emit_punctuation(current, here, tokens);
    } else if (is_whitespace(current)) {
      if (!flush()) {
        return false;
      }
    } else if (is_identifier_character(current)) {
      if (buffer.empty()) {
        buffer_loc = here;
      }
      buffer += current;
    } else if (is_opening_delimiter(current)) {
      if (!flush()) {
        return false;
      }

      delimiters.push({current, here});
      emit_punctuation(current, here, tokens);
    } else if (is_closing_delimiter(current)) {
      if (!flush()) {
        return false;
      }

      if (delimiters.empty() ||
          !delimiters_match(delimiters.top().character, current)) {
        error_at(here) << "mismatched closing delimiter: " << current << '\n';
        return false;
      }

      delimiters.pop();
      emit_punctuation(current, here, tokens);
    } else if (current == '=') {
      if (!flush()) {
        return false;
      }

      if (next_is_equal(source, i)) {
        tokens.push_back({token_type::OPERATOR, "==", here});
        step(); // consume the first '='; the loop-end step() eats the second
      } else {
        emit_punctuation(current, here, tokens);
      }
    } else if (current == ',') {
      if (!flush()) {
        return false;
      }

      emit_punctuation(current, here, tokens);
    } else if (is_operator(current)) {
      if (!flush()) {
        return false;
      }

      const loc start = here;
      std::string text(1, current);
      if ((current == '<' || current == '>' || current == '!') &&
          next_is_equal(source, i)) {
        text += '=';
        step();
      }

      tokens.push_back({token_type::OPERATOR, text, start});
    } else {
      error_at(here) << "unexpected character: " << current << '\n';
      return false;
    }

    step();
  }

  if (in_string) {
    // Point at the opening quote (one column before the content start).
    error_at({string_loc.row, string_loc.col - 1})
        << "unterminated string literal\n";
    return false;
  }

  if (!flush()) {
    return false;
  }

  if (!delimiters.empty()) {
    error_at(delimiters.top().location)
        << "unclosed delimiter: " << delimiters.top().character << '\n';
    return false;
  }

  for (const auto &x : tokens) {
    std::cout << x << '\n';
  }
  return true;
}

} // namespace eclaire::lexer
