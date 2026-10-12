#include "parser.h"
#include "lexer.h"
#include <charconv>
#include <cstdlib>
#include <iostream>

namespace eclaire {
namespace {

[[noreturn]] void error(const std::string &msg) {
  std::cerr << "Error: " << msg << '\n';
  std::exit(1);
}

int precedence(const std::string &op) {
  if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" ||
      op == ">=")
    return 1;
  if (op == "+" || op == "-")
    return 2;
  if (op == "*" || op == "/")
    return 3;
  return 0;
}

binary_op to_binary_op(const std::string &op) {
  if (op == "+")
    return binary_op::add;
  if (op == "-")
    return binary_op::sub;
  if (op == "*")
    return binary_op::mul;
  if (op == "/")
    return binary_op::div;
  if (op == "==")
    return binary_op::eq;
  if (op == "!=")
    return binary_op::ne;
  if (op == "<")
    return binary_op::lt;
  if (op == "<=")
    return binary_op::le;
  if (op == ">")
    return binary_op::gt;
  if (op == ">=")
    return binary_op::ge;
  error("unknown operator '" + op + "'");
}

class parser {
public:
  explicit parser(const std::vector<token> &input) {
    for (const token &t : input)
      if (t.type != token_type::NEWLINE)
        tokens.push_back(t);
  }

  void parse(program &p) {
    while (!at_end()) {
      if (check(token_type::FN) || check(token_type::SPELL))
        p.functions.push_back(parse_function());
      else
        error("expected 'fn' or 'spell' at top level, found '" + peek().text +
              "'");
    }
  }

private:
  std::vector<token> tokens;
  std::size_t count = 0;

  bool at_end() const { return count >= tokens.size(); }

  void advance() { count++; }

  const token &peek() const {
    if (at_end())
      error("unexpected end of input");
    return tokens[count];
  }

  bool check(token_type t) const {
    return !at_end() && tokens[count].type == t;
  }

  bool check_next(token_type t) const {
    return count + 1 < tokens.size() && tokens[count + 1].type == t;
  }

  bool match(token_type t) {
    if (!check(t))
      return false;
    advance();
    return true;
  }

  const token &expect(token_type t) {
    if (!check(t)) {
      std::string found = "end of input";
      if (!at_end())
        found = "'" + tokens[count].text + "'";
      error(std::string("expected ") + token_type_to_string(t) + ", found " +
            found);
    }
    return tokens[count++];
  }

  std::unique_ptr<function_node> parse_function() {
    auto fn = std::make_unique<function_node>();
    fn->is_spell = check(token_type::SPELL);
    advance();
    fn->name = expect(token_type::IDENTIFIER).text;
    expect(token_type::L_PAREN);
    if (!check(token_type::R_PAREN)) {
      do {
        parameter param;
        param.type = expect(token_type::TYPE).text;
        param.name = expect(token_type::IDENTIFIER).text;
        fn->parameters.push_back(std::move(param));
      } while (match(token_type::COMMA));
    }
    expect(token_type::R_PAREN);
    fn->body = parse_block();
    return fn;
  }

  block parse_block() {
    block statements;
    expect(token_type::L_BRACE);
    while (!check(token_type::R_BRACE)) {
      if (at_end())
        error("missing '}'");
      statements.push_back(parse_statement());
    }
    expect(token_type::R_BRACE);
    return statements;
  }

  stmt_ptr parse_statement() {
    switch (peek().type) {
    case token_type::LET:
      return parse_let();
    case token_type::RELEASE:
      return parse_release();
    case token_type::IF:
      return parse_if();
    case token_type::WHILE:
      return parse_while();
    case token_type::IDENTIFIER:
      if (check_next(token_type::EQUAL))
        return parse_assign();
      return parse_expr_stmt();
    default:
      error("unexpected '" + peek().text + "' at start of statement");
    }
  }

  stmt_ptr parse_let() {
    auto node = std::make_unique<let_stmt>();
    expect(token_type::LET);
    node->name = expect(token_type::IDENTIFIER).text;
    expect(token_type::EQUAL);
    node->value = parse_expression();
    return node;
  }

  stmt_ptr parse_assign() {
    auto node = std::make_unique<assign_stmt>();
    node->name = expect(token_type::IDENTIFIER).text;
    expect(token_type::EQUAL);
    node->value = parse_expression();
    return node;
  }

  stmt_ptr parse_release() {
    auto node = std::make_unique<release_stmt>();
    expect(token_type::RELEASE);
    node->value = parse_expression();
    return node;
  }

  stmt_ptr parse_expr_stmt() {
    auto node = std::make_unique<expr_stmt>();
    node->value = parse_expression();
    return node;
  }

  stmt_ptr parse_if() {
    auto node = std::make_unique<if_stmt>();
    expect(token_type::IF);
    branch first;
    first.condition = parse_expression();
    first.body = parse_block();
    node->branches.push_back(std::move(first));
    while (match(token_type::ELIF)) {
      branch arm;
      arm.condition = parse_expression();
      arm.body = parse_block();
      node->branches.push_back(std::move(arm));
    }
    if (match(token_type::ELSE))
      node->else_body = parse_block();
    return node;
  }

  stmt_ptr parse_while() {
    auto node = std::make_unique<while_stmt>();
    expect(token_type::WHILE);
    node->condition = parse_expression();
    node->body = parse_block();
    return node;
  }

  expr_ptr parse_expression(int min_prec = 1) {
    expr_ptr left = parse_primary();
    while (check(token_type::OPERATOR) && precedence(peek().text) >= min_prec) {
      std::string op = peek().text;
      advance();
      expr_ptr right = parse_expression(precedence(op) + 1);
      auto node = std::make_unique<binary_expr>();
      node->op = to_binary_op(op);
      node->left = std::move(left);
      node->right = std::move(right);
      left = std::move(node);
    }
    return left;
  }

  expr_ptr parse_primary() {
    switch (peek().type) {
    case token_type::INTEGER:
      return parse_integer();
    case token_type::INVOKE: {
      advance();
      auto node = std::make_unique<invoke_expr>();
      node->call = parse_call();
      return node;
    }
    case token_type::IDENTIFIER:
      if (check_next(token_type::L_PAREN))
        return parse_call();
      return parse_variable();
    case token_type::L_PAREN: {
      advance();
      expr_ptr inner = parse_expression();
      expect(token_type::R_PAREN);
      return inner;
    }
    default:
      error("unexpected '" + peek().text + "' in expression");
    }
  }

  expr_ptr parse_integer() {
    const std::string &text = peek().text;
    std::int64_t value = 0;
    auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc() || ptr != text.data() + text.size())
      error("invalid or out of range integer '" + text + "'");
    advance();
    auto node = std::make_unique<integer_expr>();
    node->value = value;
    return node;
  }

  expr_ptr parse_variable() {
    auto node = std::make_unique<variable_expr>();
    node->name = expect(token_type::IDENTIFIER).text;
    return node;
  }

  std::unique_ptr<call_expr> parse_call() {
    auto call = std::make_unique<call_expr>();
    call->name = expect(token_type::IDENTIFIER).text;
    expect(token_type::L_PAREN);
    if (!check(token_type::R_PAREN)) {
      do {
        call->arguments.push_back(parse_expression());
      } while (match(token_type::COMMA));
    }
    expect(token_type::R_PAREN);
    return call;
  }
};

} // namespace

program parse(const std::vector<token> &tokens) {
  program root;
  parser p(tokens);
  p.parse(root);
  return root;
}

} // namespace eclaire
