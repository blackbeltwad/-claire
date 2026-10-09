#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <string>
#include <vector>
namespace eclaire {

class ast_node {
public:
  virtual ~ast_node() = default;
};

class expression : public ast_node {
public:
  virtual ~expression() = default;
};

struct parameter {
  std::string type;
  std::string name;
};

class function_node : public ast_node {
public:
  std::string name;
  std::vector<std::unique_ptr<parameter>> parameters;
  std::vector<std::unique_ptr<ast_node>> body_nodes;
};

class let_node : public ast_node {
public:
  std::string name;
  std::unique_ptr<expression> value;
};

class release_node : public ast_node {
public:
  std::unique_ptr<expression> value;
};

class function_call_node : public expression {
public:
  std::string name;
  std::vector<std::unique_ptr<expression>> arguments;
};

class integer_expr : public expression {
public:
  int value;
};

class variable_expr : public expression {
public:
  std::string name;
};

class binary_expr : public expression {
public:
  std::unique_ptr<expression> left;
  std::unique_ptr<expression> right;
  std::string op;
};

class parser {
private:
  const std::vector<token> &tokens;
  std::size_t count;

  void advance() { count++; }

  std::unique_ptr<parameter> consume_type_identifier() {
    auto unique_parameter = std::make_unique<parameter>();
    unique_parameter->type = tokens[count].text;
    advance(); // Consume type
    unique_parameter->name = tokens[count].text;
    return unique_parameter;
  }

  void consume_name(function_node &fn_node) {
    fn_node.name = tokens[count].text;
    advance();
  }

  void consume_name(std::unique_ptr<let_node> &l_node) {
    l_node->name = tokens[count].text;
    advance();
  }

  const token &peek() const { return tokens[count]; }

public:
  explicit parser(const std::vector<token> &input) : tokens(input) {};

  void parse_function(function_node &node) {
    advance();          // Consume FN
    consume_name(node); // Consume NAME
    advance();          // Consume LPAR

    while (tokens[count].type != token_type::R_PAREN) {
      node.parameters.push_back(consume_type_identifier());
      advance();
      if (peek().type == token_type::COMMA) {
        advance();
      }
    }

    advance();
    while (tokens[count].type != token_type::R_BRACKET) {
      if (tokens[count].type == token_type::LET) {
      }
      if (tokens[count].type == token_type::RELEASE) {
      }
    }
  }

  std::unique_ptr<ast_node> parse_let() {
    advance(); // Consume LET
    auto node = std::make_unique<let_node>();
    consume_name(node); // Consume Name
    advance();          // Consume Equal

    return node;
  }
};
} // namespace eclaire
#endif
