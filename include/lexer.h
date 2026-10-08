#ifndef LEXER_H
#define LEXER_H
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

  // DISJOINTS
  COMMA,
  L_PAREN,
  R_PAREN,
  L_BRACKET,
  R_BRACKET,
  EQUAL,
  OPERATOR,
};

struct token {
  token_type type;
  std::string text;
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
