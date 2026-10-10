#include "parser.h"
#include "lexer.h"
#include <iostream>
namespace eclaire {
namespace {
[[noreturn]] void error(std::string &msg) {
  std::cerr << "Error: " << msg << '\n';
  std::exit(1);
}

class parser {
private:
  const std::vector<token> &tokens;
  std::size_t count;

  void advance() { count++; }

  parameter consume_type_identifier() {
    parameter parameter_t;
    parameter_t.type = tokens[count].text;
    advance(); // Consume type
    parameter_t.name = tokens[count].text;
    advance();
    return parameter_t;
  }

  void consume_name(std::unique_ptr<function_node> &fn_node) {
    fn_node->name = tokens[count].text;
    advance();
  }

  void consume_name(std::unique_ptr<let_stmt> &l_node) {
    l_node->name = tokens[count].text;
    advance();
  }

  const token &peek() const { return tokens[count]; }

public:
  explicit parser(const std::vector<token> &input) : tokens(input), count(0) {};

  void parse(program &p) {
    while (count < tokens.size()) {
      if (tokens[count].type == token_type::FN ||
          tokens[count].type == token_type::SPELL) {
        p.functions.push_back(parse_function(tokens[count].type));
      }
      advance();
    }
  }
  std::unique_ptr<function_node> parse_function(token_type type) {
    auto f_node = std::make_unique<function_node>();
    if (type == token_type::SPELL) {
      f_node->is_spell = true;
    }
    advance();            // Consume FN
    consume_name(f_node); // Consume NAME
    advance();            // Consume LPAR

    // Consume the parameters
    while (tokens[count].type != token_type::R_PAREN) {
      f_node->parameters.push_back(consume_type_identifier());
      if (peek().type == token_type::COMMA) {
        advance();
      }
    }

    advance(); // Consume RPAREN
    while (tokens[count].type != token_type::R_BRACKET) {
      if (tokens[count].type == token_type::LET) {
      }
      if (tokens[count].type == token_type::RELEASE) {
      }
    }

    return f_node;
  }

  std::unique_ptr<ast_node> parse_let() {}
};
} // namespace
program parse(const std::vector<token> &tokens) {
  program root;
  parser tree(tokens);
  tree.parse(root);
  return root;
}
} // namespace eclaire
