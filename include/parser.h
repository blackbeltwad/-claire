#ifndef ECLAIRE_PARSER_H
#define ECLAIRE_PARSER_H

#include "source_loc.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace eclaire::parser {

// ---------- Expressions ----------

enum class kind : char {
  // Num and Var are leaves
  // Neg and Not are unary
  // Call is n-ary operands are the arguments
  // Everything else is binary
  Num,
  Var,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  And,
  Or,
  GT,
  GTOET,
  LT,
  LTOET,
  Equal,
  NE,
  Neg,
  Not,
  Call
};

enum class type : char { int_32, int_64, uint_32, uint_64 };

struct expr_nry;
using expr_p = std::unique_ptr<expr_nry>;

struct expr_nry {
  kind _kind;
  std::vector<expr_p> operands;
  std::string name;
  std::int64_t value = 0;
  bool compile_time = false;
  loc location;
};

// ---------- Statements ----------

struct let_stmt;
struct assign_stmt;
struct release_stmt;
struct expr_stmt;
struct if_stmt;
struct while_stmt;

// One statement type for everything. Function, if and while bodies
// are all just std::vector<stmt>.
using stmt = std::variant<let_stmt, assign_stmt, release_stmt, expr_stmt,
                          if_stmt, while_stmt>;

struct variable {
  type _type;
  std::string name;
  loc location;
};

struct let_stmt {
  variable var;
  expr_p expression;
};

struct assign_stmt {
  std::string name;
  expr_p expression;
  loc location;
};

struct release_stmt {
  expr_p expression;
  loc location;
};

struct expr_stmt {
  expr_p expression;
  loc location;
};

struct if_stmt {
  struct branch {
    expr_p condition;
    std::vector<stmt> body;
  };
  std::vector<branch> branches;
  std::optional<std::vector<stmt>> else_body;
  loc location;
};

struct while_stmt {
  expr_p condition;
  std::vector<stmt> body;
  loc location;
};

// ---------- Top level ----------

struct fn {
  std::string name;
  type return_type;
  std::vector<variable> parameters;
  std::vector<stmt> body;
  bool is_spell = false; // declared with spell is compile time function
  loc location;
  fn(std::string _name, bool _b, loc _source)
      : name(_name), is_spell(_b), location(_source) {}
};

struct program {
  std::vector<fn> functions;
};

} // namespace eclaire::parser

#endif
