#ifndef ECLAIRE_LEXER_H
#define ECLAIRE_LEXER_H

#include <iosfwd>
#include <string>
#include <vector>

namespace eclaire {

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
  COMMA,
  L_PAREN,
  R_PAREN,
  L_BRACE,
  R_BRACE,
  L_BRACKET,
  R_BRACKET,
  EQUAL,
  OPERATOR,
  QUOTE,
  NEWLINE
};

struct token {
  token_type type;
  std::string text;
};

const char *token_type_to_string(token_type type);
std::ostream &operator<<(std::ostream &os, const token &current_token);

bool tokenize(const std::string &source, std::vector<token> &tokens);

} // namespace eclaire

#endif
