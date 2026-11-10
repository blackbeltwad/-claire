#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"

using namespace eclaire;
using parser::expr_nry;
using parser::kind;
namespace {

std::string get_operator_name(kind k) {
  switch (k) {
  case kind::Add:
    return "+";
  case kind::Sub:
    return "-";
  case kind::Mul:
    return "*";
  case kind::Div:
    return "/";
  case kind::Mod:
    return "%";
  case kind::And:
    return "&&";
  case kind::Or:
    return "||";
  case kind::GT:
    return ">";
  case kind::GTOET:
    return ">=";
  case kind::LT:
    return "<";
  case kind::LTOET:
    return "<=";
  case kind::Equal:
    return "==";
  case kind::NE:
    return "!=";
  case kind::Neg:
    return "neg";
  case kind::Not:
    return "not";
  default:
    return "?";
  }
}

std::string dump(const expr_nry &root) {
  switch (root._kind) {
  case kind::Num:
    return std::to_string(root.value);
  case kind::Var:
    return root.name;
  case kind::Call: {
    std::string s = root.name;
    s += "(";
    for (std::size_t i = 0; i < root.operands.size(); i++) {
      s += dump(*root.operands[i]);
      if (i != root.operands.size() - 1) {
        s += ", ";
      }
    }

    return s + ")";
  }
  default:
    break;
  }
  if (root._kind == kind::Neg) {
    return "(-" + dump(*root.operands[0]) + ")";
  }
  if (root._kind == kind::Not) {
    return "(!" + dump(*root.operands[0]) + ")";
  }

  std::string buffer;
  buffer += "(";
  for (std::size_t i = 0; i < root.operands.size(); i++) {
    buffer += dump(*root.operands[i]);
    if (i != root.operands.size() - 1) {
      buffer += " " + get_operator_name(root._kind) + " ";
    }
  }
  buffer += ")";

  return buffer;
}

std::string parse_expr(const std::string &expr) {
  const std::string source = "fn main() { let x = " + expr + " }";

  std::vector<lexer::token> tokens;
  EXPECT_TRUE(lexer::tokenize(source, tokens));

  parser::program prog = parser::parse_to_ast(tokens);
  const auto &let = std::get<parser::let_stmt>(prog.functions.at(0).body.at(0));
  return dump(*let.expression);
}
} // namespace

TEST(ParseExpression, Literals) {
  EXPECT_EQ(parse_expr("42"), "42");
  EXPECT_EQ(parse_expr("x"), "x");
}

TEST(ParseExpression, MulBindsTighterThanAdd) {
  EXPECT_EQ(parse_expr("1 + 2 * 3"), "(1 + (2 * 3))");
  EXPECT_EQ(parse_expr("1 * 2 + 3"), "((1 * 2) + 3)");
}

TEST(ParseExpression, LeftAssociative) {
  EXPECT_EQ(parse_expr("1 - 2 - 3"), "((1 - 2) - 3)");
  EXPECT_EQ(parse_expr("8 / 4 / 2"), "((8 / 4) / 2)");
}

TEST(ParseExpression, Parentheses) {
  EXPECT_EQ(parse_expr("(1 + 2) * 3"), "((1 + 2) * 3)");
  EXPECT_EQ(parse_expr("((1))"), "1");
}

TEST(ParseExpression, Unary) {
  EXPECT_EQ(parse_expr("-x"), "(-x)");
  EXPECT_EQ(parse_expr("!-x"), "(!(-x))");
  EXPECT_EQ(parse_expr("-a * b"), "((-a) * b)");
}

TEST(ParseExpression, Calls) {
  EXPECT_EQ(parse_expr("f()"), "f()");
  EXPECT_EQ(parse_expr("f(1, 2 + 3)"), "f(1, (2 + 3))");
  EXPECT_EQ(parse_expr("f(g(1), (2))"), "f(g(1), 2)");
}
