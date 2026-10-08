#ifndef LEXER_H
#define LEXER_H
#include <ostream>
#include <string>
#include <unordered_map>

enum class token_type {
  FN,
  SPELL,
  INVOKE,
  RELEASE,
  LET,

  IDENTIFIER,
  INTEGER,
  STRING,
  TYPE,

  // DISJOINTS
  COMMA,
  L_PAREN,
  R_PAREN,
  L_BRACKET,
  R_BRACKET,
  EQUAL,
  OPERATOR,
  SINGLE_QUOTE
};

static const char *token_type_to_string(token_type type) {
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
  case token_type::L_BRACKET:
    return "L_BRACKET";
  case token_type::R_BRACKET:
    return "R_BRACKET";
  case token_type::EQUAL:
    return "EQUAL";
  case token_type::OPERATOR:
    return "OPERATOR";
  case token_type::SINGLE_QUOTE:
    return "SINGLE_QUOTE";
  }

  return "UNKNOWN";
}

struct token {
  token_type type;
  std::string text;

  friend std::ostream &operator<<(std::ostream &os, const token &t) {
    return os << "{" << token_type_to_string(t.type) << ": \"" << t.text
              << "\"}";
  }
};

static std::unordered_map<std::string, token_type> keyvalues{
    {"fn", token_type::FN},           {"let", token_type::LET},
    {"spell", token_type::SPELL},     {"invoke", token_type::INVOKE},
    {"release", token_type::RELEASE}, {",", token_type::COMMA},
    {"(", token_type::L_PAREN},       {")", token_type::R_PAREN},
    {"{", token_type::L_BRACKET},     {"}", token_type::R_BRACKET},
    {"=", token_type::EQUAL},         {"+", token_type::OPERATOR},
    {"-", token_type::OPERATOR},      {"1", token_type::INTEGER},
    {"a", token_type::STRING},        {"-", token_type::OPERATOR},
    {"*", token_type::OPERATOR},      {"/", token_type::OPERATOR},
    {"`", token_type::SINGLE_QUOTE},  {"int", token_type::TYPE},
    {"string", token_type::TYPE},

};

// helper functions
static bool is_whitespace(char character) {
  return std::isspace(static_cast<unsigned char>(character));
}

static bool is_digit(char character) {
  return std::isdigit(static_cast<unsigned char>(character));
}

static bool is_letter(char character) {
  return std::isalpha(static_cast<unsigned char>(character));
}

static bool is_identifier_start(char character) {
  return is_letter(character) || character == '_';
}

static bool is_identifier_character(char character) {
  return is_letter(character) || is_digit(character) || character == '_';
}

static bool is_opening_bracket(char character) { return character == '['; }

static bool is_closing_bracket(char character) { return character == ']'; }

static bool is_opening_parenthesis(char character) { return character == '('; }

static bool is_closing_parenthesis(char character) { return character == ')'; }

static bool matching_delimiter(char opening, char closing) {
  return (opening == '[' && closing == ']') ||
         (opening == '(' && closing == ')');
}

static bool is_comma(char character) { return character == ','; }

#endif
