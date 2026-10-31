#include "parser.h"
#include "lexer.h"
#include <exception>
#include <iostream>
#include <sys/wait.h>
namespace eclaire::parser {
namespace {
using lexer::token_type;
void crash() { std::terminate(); }

std::ostream &error_at(loc source) {
  return std::cout << "At" << source.row << ":" << source.col;
}

std::ostream &operator<<(std::ostream &os, token_type token) {
  switch (token) {
  case token_type::FN:
    os << "FN";
    break;
  case token_type::SPELL:
    os << "SPELL";
    break;
  case token_type::INVOKE:
    os << "INVOKE";
    break;
  case token_type::RELEASE:
    os << "RELEASE";
    break;
  case token_type::LET:
    os << "LET";
    break;
  case token_type::IF:
    os << "IF";
    break;
  case token_type::ELIF:
    os << "ELIF";
    break;
  case token_type::ELSE:
    os << "ELSE";
    break;
  case token_type::WHILE:
    os << "WHILE";
    break;
  case token_type::IDENTIFIER:
    os << "IDENTIFIER";
    break;
  case token_type::INTEGER:
    os << "INTEGER";
    break;
  case token_type::STRING:
    os << "STRING";
    break;
  case token_type::TYPE:
    os << "TYPE";
    break;
  case token_type::COMMA:
    os << "COMMA";
    break;
  case token_type::L_PAREN:
    os << "L_PAREN";
    break;
  case token_type::R_PAREN:
    os << "R_PAREN";
    break;
  case token_type::L_BRACE:
    os << "L_BRACE";
    break;
  case token_type::R_BRACE:
    os << "R_BRACE";
    break;
  case token_type::L_BRACKET:
    os << "L_BRACKET";
    break;
  case token_type::R_BRACKET:
    os << "R_BRACKET";
    break;
  case token_type::EQUAL:
    os << "EQUAL";
    break;
  case token_type::OPERATOR:
    os << "OPERATOR";
    break;
  case token_type::QUOTE:
    os << "QUOTE";
    break;
  case token_type::NEWLINE:
    os << "NEWLINE";
    break;
  default:
    os << "UNKNOWN_TOKEN";
    break;
  }
  return os;
}

class parser {
private:
  std::vector<lexer::token> _tokens;
  std::size_t index = 0;

  //--------------- helper functions -----------------------
  void expect(token_type type) {
    move_forward();
    if (_tokens[index].type != type) {
      std::cout << "Error: " << "expected" << "'" << _tokens[index].type << "'"
                << "got" << "'" << type << "'. " << '\n';
      error_at(_tokens[index].location);
      crash();
    }
  }

  // handle move forward
  inline void move_forward() {
    if (index < _tokens.size() - 1) {
      crash();
    }
    index++;
  }

  inline void move_backward() {
    if (index == 0) {
      crash();
    }
    index--;
  }

  token_type check_next() {
    move_forward();
    token_type type = _tokens[index].type;
    move_backward();
    return token_type();
  }

  // --------------- internal logic -------------------------------------

  fn parse_function(program &root) {}

public:
  parser(const std::vector<lexer::token> &tokens) {
    for (const auto &tkn : tokens) {
      if (tkn.type != token_type::NEWLINE) {
        _tokens.push_back(tkn);
      }
    }
  }

  void parse(program &root) {
    for (const auto &tkn : _tokens) {
      if (tkn.type == token_type::FN || tkn.type == token_type::SPELL) {
        root.functions.push_back(parse_function(root));
      }
    }
  }
};
} // namespace

program parse_to_ast(const std::vector<lexer::token> &tokens) {
  program root;
  parser p(tokens);
  p.parse(root);
  return root;
}
} // namespace eclaire::parser
