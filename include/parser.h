#ifndef ECLAIRE_PARSER_H
#define ECLAIRE_PARSER_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace eclaire {

enum class kind {
  integer,
  variable,
  binary,
  call,
  invoke,
  let,
  assign,
  release,
  if_stmt,
  while_stmt,
  expr_stmt,
  function
};

enum class binary_op { add, sub, mul, div, eq, ne, lt, le, gt, ge };

struct ast_node {
  explicit ast_node(kind k) : node_kind(k) {}
  virtual ~ast_node() = default;
  kind node_kind;
};

struct expression : ast_node {
  using ast_node::ast_node;
};
struct statement : ast_node {
  using ast_node::ast_node;
};

using expr_ptr = std::unique_ptr<expression>;
using stmt_ptr = std::unique_ptr<statement>;
using block = std::vector<stmt_ptr>;

struct integer_expr : expression {
  integer_expr() : expression(kind::integer) {}
  std::int64_t value = 0;
};

struct variable_expr : expression {
  variable_expr() : expression(kind::variable) {}
  std::string name;
};

struct binary_expr : expression {
  binary_expr() : expression(kind::binary) {}
  binary_op op = binary_op::add;
  expr_ptr left, right;
};

struct call_expr : expression {
  call_expr() : expression(kind::call) {}
  std::string name;
  std::vector<expr_ptr> arguments;
};

struct invoke_expr : expression {
  invoke_expr() : expression(kind::invoke) {}
  std::unique_ptr<call_expr> call;
};

struct let_stmt : statement {
  let_stmt() : statement(kind::let) {}
  std::string name;
  expr_ptr value;
};

struct assign_stmt : statement {
  assign_stmt() : statement(kind::assign) {}
  std::string name;
  expr_ptr value;
};

struct release_stmt : statement {
  release_stmt() : statement(kind::release) {}
  expr_ptr value;
};

struct expr_stmt : statement {
  expr_stmt() : statement(kind::expr_stmt) {}
  expr_ptr value;
};

struct branch {
  expr_ptr condition;
  block body;
};

struct if_stmt : statement {
  if_stmt() : statement(kind::if_stmt) {}
  std::vector<branch> branches;
  block else_body;
};

struct while_stmt : statement {
  while_stmt() : statement(kind::while_stmt) {}
  expr_ptr condition;
  block body;
};

struct parameter {
  std::string type, name;
};

struct function_node : ast_node {
  function_node() : ast_node(kind::function) {}
  std::string name;
  std::vector<parameter> parameters;
  block body;
  bool is_spell = false;
};

} // namespace eclaire

#endif
