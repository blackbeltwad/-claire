#ifndef ECLAIRE_PARSER_H
#define ECLAIRE_PARSER_H
#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace eclaire::parser {
// Declarations
struct program;
struct fn;
struct if_stmt;
struct while_stmt;
struct stmts;
struct expr_nry;
struct let_stmt;
struct assign_stmt;
struct return_stmt;
struct variable;

enum class kind : char {
  // Num and Var are leaves
  // Everything else is binary
  Num,
  Var,
  Add,
  Mul,
  Sub,
  Div,
  And,
  Or,
  GT,
  GTOET,
  LT,
  LTOET,
  Equal,
  Call,
  Neg,
  Not,
  NE
};

enum class type : char { int_32, int_64, uint_32, uint_64 };

using expr_stmt = std::variant<let_stmt, assign_stmt, return_stmt>;
using block = std::variant<if_stmt, while_stmt, stmts>;

struct program {
  std::vector<fn> functions;
};

struct variable {
  type _type;
  std::string name;
  std::size_t line;
  std::size_t col;
};

struct fn {
  std::string name;
  type _type;
  std::vector<variable> parameters;
  std::vector<block> blocks;
  std::size_t line;
  std::size_t col;
};

using expr_p = std::unique_ptr<expr_nry>;

struct expr_nry {
  kind _kind;
  std::vector<expr_p> operands;
  std::string name;
  size_t value;
  std::size_t line;
  std::size_t col;
};

struct stmts {
  std::vector<expr_stmt> expressions;
  std::size_t line;
  std::size_t col;
};
struct let_expr {
  std::string name;
  type _type;
  expr_p expression;
  std::size_t line;
  std::size_t col;
};

struct assign_expr {
  std::string name;
  expr_p expression;
  std::size_t line;
  std::size_t col;
};

struct return_expr {
  expr_p expression;
  std::size_t line;
  std::size_t col;
};

} // namespace eclaire::parser
#endif
