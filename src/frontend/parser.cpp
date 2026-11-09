#include "parser.h"
#include "lexer.h"
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace eclaire::parser {
namespace {
using lexer::token;
using lexer::token_type;

[[noreturn]] void crash() {
  // For now i have no clue what is better to say here tbh
  std::terminate();
}

// Same "Error: row:col: " format as the lexer, and on stderr.
std::ostream &error_at(loc source) {
  return std::cerr << "Error: " << source.row << ':' << source.col << ": ";
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
  // Convention: index is the CURRENT / LAST CONSUMED token.
  std::size_t index = 0;

  //--------------- helper functions -----------------------

  // Next token without consuming it
  const token *peek() const {
    return index + 1 < _tokens.size() ? &_tokens[index + 1] : nullptr;
  }

  // precedence values
  static int binary_precedence(const std::string &op) {
    if (op == "||")
      return 1;
    if (op == "&&")
      return 2;
    if (op == "==" || op == "!=")
      return 3;
    if (op == "<" || op == "<=" || op == ">" || op == ">=")
      return 4;
    if (op == "+" || op == "-")
      return 5;
    if (op == "*" || op == "/" || op == "%")
      return 6;
    return -1;
  }

  static kind binary_kind(const std::string &op) {
    if (op == "||")
      return kind::Or;
    if (op == "&&")
      return kind::And;
    if (op == "==")
      return kind::Equal;
    if (op == "!=")
      return kind::NE;
    if (op == "<")
      return kind::LT;
    if (op == "<=")
      return kind::LTOET;
    if (op == ">")
      return kind::GT;
    if (op == ">=")
      return kind::GTOET;
    if (op == "+")
      return kind::Add;
    if (op == "-")
      return kind::Sub;
    if (op == "*")
      return kind::Mul;
    if (op == "/")
      return kind::Div;
    return kind::Mod; // "%"
  }

  void unexpected_end() {
    if (!_tokens.empty()) {
      error_at(_tokens.back().location);
    }
    std::cerr << "unexpected end of input\n";
    crash();
  }

  // Consume the next token and make sure it has the given type.
  void expect(token_type type) {
    move_forward();
    if (_tokens[index].type != type) {
      error_at(_tokens[index].location)
          << "expected '" << type << "', got '" << _tokens[index].type << "'\n";
      crash();
    }
  }

  inline void move_forward() {
    if (index + 1 >= _tokens.size()) {
      unexpected_end();
    }
    index++;
  }

  inline void move_backward() {
    if (index == 0) {
      crash();
    }
    index--;
  }

  // Type of the next token, without consuming it.
  token_type check_next() {
    if (index + 1 >= _tokens.size()) {
      unexpected_end();
    }
    return _tokens[index + 1].type;
  }

  token_type current_token_type() { return _tokens[index].type; }
  token current_token() { return _tokens[index]; }

  // --------------- internal logic -------------------------------------

  // primary := INTEGER | IDENT | IDENT '(' [expr {',' expr}] ')' | '(' expr ')'
  expr_p parse_primary() {
    const token *t = peek();
    if (t == nullptr) {
      unexpected_end();
    }

    switch (t->type) {
    case token_type::INTEGER: {
      move_forward();
      token tok = current_token();
      auto node = std::make_unique<expr_nry>();
      node->_kind = kind::Num;
      node->location = tok.location;
      node->value = std::stoll(tok.text);
      return node;
    }
    case token_type::IDENTIFIER: {
      move_forward();
      token tok = current_token();
      auto node = std::make_unique<expr_nry>();
      node->location = tok.location;
      node->name = tok.text;

      if (check_next() == token_type::L_PAREN) {
        move_forward(); // '('
        node->_kind = kind::Call;
        if (check_next() != token_type::R_PAREN) {
          while (true) {
            node->operands.push_back(parse_expression());
            if (check_next() == token_type::COMMA) {
              move_forward();
              continue;
            }
            break;
          }
        }
        expect(token_type::R_PAREN);
      } else {
        node->_kind = kind::Var;
      }
      return node;
    }
    case token_type::L_PAREN: {
      move_forward();
      expr_p inner = parse_expression();
      expect(token_type::R_PAREN);
      return inner;
    }
    default:
      error_at(t->location) << "expected expression, got '" << t->type << "'\n";
      crash();
    }
  }

  // unary := ('-' | '!') unary | primary
  expr_p parse_unary() {
    const token *t = peek();
    if (t && t->type == token_type::OPERATOR &&
        (t->text == "-" || t->text == "!")) {
      move_forward();
      token op = current_token();
      auto node = std::make_unique<expr_nry>();
      node->_kind = (op.text == "-") ? kind::Neg : kind::Not;
      node->location = op.location;
      node->operands.push_back(parse_unary());
      return node;
    }
    return parse_primary();
  }

  // Precedence climbing, all binary operators left associative.
  expr_p parse_expression(int min_prec = 1) {
    expr_p lhs = parse_unary();

    while (true) {
      const token *t = peek();
      if (t == nullptr || t->type != token_type::OPERATOR) {
        break;
      }

      const int prec = binary_precedence(t->text);
      if (prec < min_prec) {
        break;
      }

      move_forward();
      token op = current_token();
      expr_p rhs = parse_expression(prec + 1);

      auto node = std::make_unique<expr_nry>();
      node->_kind = binary_kind(op.text);
      node->location = op.location;
      node->operands.push_back(std::move(lhs));
      node->operands.push_back(std::move(rhs));
      lhs = std::move(node);
    }

    return lhs;
  }

  stmt parse_let() {
    expect(token_type::IDENTIFIER);
    token name = current_token();
    expect(token_type::EQUAL);

    let_stmt s;
    s.var.name = name.text;
    s.var.location = name.location;
    s.var._type = type::int_32; // TODO: infer/etc
    s.expression = parse_expression();
    return stmt{std::move(s)};
  }

  // Called with index on the token BEFORE the statement.
  stmt parse_stmts() {
    move_forward();

    switch (current_token_type()) {
    case token_type::LET:
      return parse_let();
    default:
      error_at(current_token().location)
          << "unexpected token '" << current_token_type()
          << "' at start of statement\n";
      crash();
    }
  }

  // Called with index on the FN / SPELL token. Returns with index on '}'.
  fn parse_function() {
    const bool is_spell = current_token_type() == token_type::SPELL;

    expect(token_type::IDENTIFIER);
    token fn_token = current_token();

    fn func(fn_token.text, is_spell, fn_token.location);

    expect(token_type::L_PAREN);

    if (check_next() != token_type::R_PAREN) {
      while (true) {
        variable param;
        expect(token_type::TYPE);
        token param_type = current_token();

        // this will upgrade to a switch statement eventually
        if (param_type.text == "int") {
          param._type = type::int_32;
        } else {
          error_at(param_type.location)
              << "unsupported type: " << param_type.text << '\n';
          crash();
        }

        expect(token_type::IDENTIFIER);
        token param_name = current_token();
        param.name = param_name.text;
        param.location = param_name.location;

        func.parameters.push_back(std::move(param));

        if (check_next() == token_type::COMMA) {
          expect(token_type::COMMA);
          continue;
        }
        break;
      }
    }
    expect(token_type::R_PAREN);

    // parse statements time
    expect(token_type::L_BRACE);
    while (check_next() != token_type::R_BRACE) {
      func.body.push_back(parse_stmts());
    }
    expect(token_type::R_BRACE);

    return func;
  }

public:
  explicit parser(const std::vector<lexer::token> &tokens) {
    for (const auto &tkn : tokens) {
      if (tkn.type != token_type::NEWLINE) {
        _tokens.push_back(tkn);
      }
    }
  }

  void parse(program &root) {
    while (index < _tokens.size()) {
      const token_type t = current_token_type();

      if (t != token_type::FN && t != token_type::SPELL) {
        error_at(current_token().location)
            << "expected 'fn' or 'spell' at top level, got '" << t << "'\n";
        crash();
      }

      root.functions.push_back(parse_function());

      // parse_function leaves us on the closing '}'.
      if (index + 1 >= _tokens.size()) {
        break;
      }
      index++;
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
