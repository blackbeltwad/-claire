
#include "lexer.h"

#include <cctype>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>

namespace eclaire {
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

bool emit_word(const std::string &word, std::vector<token> &tokens) {
  if (word.empty()) {
    return true;
  }

  const auto keyword = keywords.find(word);

  if (keyword != keywords.end()) {
    tokens.push_back({keyword->second, word});
    return true;
  }

  if (is_digit(word.front())) {
    for (char character : word) {
      if (!is_digit(character)) {
        std::cerr << "Error: invalid integer literal: " << word << '\n';
        return false;
      }
    }

    tokens.push_back({token_type::INTEGER, word});
    return true;
  }

  if (!is_identifier_start(word.front())) {
    std::cerr << "Error: invalid token: " << word << '\n';
    return false;
  }

  for (char character : word) {
    if (!is_identifier_character(character)) {
      std::cerr << "Error: invalid identifier: " << word << '\n';
      return false;
    }
  }

  tokens.push_back({token_type::IDENTIFIER, word});
  return true;
}

bool emit_punctuation(char character, std::vector<token> &tokens) {
  switch (character) {
  case ',':
    tokens.push_back({token_type::COMMA, ","});
    return true;
  case '(':
    tokens.push_back({token_type::L_PAREN, "("});
    return true;
  case ')':
    tokens.push_back({token_type::R_PAREN, ")"});
    return true;
  case '{':
    tokens.push_back({token_type::L_BRACE, "{"});
    return true;
  case '}':
    tokens.push_back({token_type::R_BRACE, "}"});
    return true;
  case '[':
    tokens.push_back({token_type::L_BRACKET, "["});
    return true;
  case ']':
    tokens.push_back({token_type::R_BRACKET, "]"});
    return true;
  case '=':
    tokens.push_back({token_type::EQUAL, "="});
    return true;
  case '\n':
    tokens.push_back({token_type::NEWLINE, " "});
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
            << current_token.text << "\"}";
}

bool tokenize(const std::string &source, std::vector<token> &tokens) {
  tokens.clear();

  std::string buffer;
  std::stack<char> delimiters;
  bool in_string = false;

  for (std::size_t i = 0; i < source.size(); ++i) {
    const char current = source[i];

    if (in_string) {
      if (current == '`') {
        tokens.push_back({token_type::STRING, buffer});
        tokens.push_back({token_type::QUOTE, "`"});
        buffer.clear();
        in_string = false;
      } else {
        buffer += current;
      }

      continue;
    }

    if (current == '`') {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
      tokens.push_back({token_type::QUOTE, "`"});
      in_string = true;
    } else if (is_new_line(current)) {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
      emit_punctuation(current, tokens);
    } else if (is_whitespace(current)) {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
    } else if (is_identifier_character(current)) {
      buffer += current;
    } else if (is_opening_delimiter(current)) {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
      delimiters.push(current);
      emit_punctuation(current, tokens);
    } else if (is_closing_delimiter(current)) {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();

      if (delimiters.empty() || !delimiters_match(delimiters.top(), current)) {
        std::cerr << "Error: mismatched closing delimiter: " << current << '\n';
        return false;
      }

      delimiters.pop();
      emit_punctuation(current, tokens);
    } else if (current == ',' || current == '=') {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
      emit_punctuation(current, tokens);
    } else if (is_operator(current)) {
      if (!emit_word(buffer, tokens)) {
        return false;
      }

      buffer.clear();
      tokens.push_back({token_type::OPERATOR, std::string(1, current)});
    } else {
      std::cerr << "Error: unexpected character: " << current << '\n';
      return false;
    }
  }

  if (in_string) {
    std::cerr << "Error: unterminated string literal\n";
    return false;
  }

  if (!emit_word(buffer, tokens)) {
    return false;
  }

  if (!delimiters.empty()) {
    std::cerr << "Error: missing closing delimiter\n";
    return false;
  }

  for (const auto &x : tokens) {
    std::cout << x << '\n';
  }
  return true;
}

} // namespace eclaire
