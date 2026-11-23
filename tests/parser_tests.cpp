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

// ---------- More expression tests ----------

TEST(ParseExpression, PrecedenceLadder) {
  // || < && < ==,!= < <,<=,>,>= < +,- < *,/,%
  EXPECT_EQ(parse_expr("a || b && c == d < e + f * g"),
            "(a || (b && (c == (d < (e + (f * g))))))");
  EXPECT_EQ(parse_expr("a * b + c < d == e && f || g"),
            "((((((a * b) + c) < d) == e) && f) || g)");
}

TEST(ParseExpression, LogicalOperators) {
  EXPECT_EQ(parse_expr("a || b && c"), "(a || (b && c))");
  EXPECT_EQ(parse_expr("a && b || c"), "((a && b) || c)");
  EXPECT_EQ(parse_expr("a || b || c"), "((a || b) || c)");
  EXPECT_EQ(parse_expr("a && b && c"), "((a && b) && c)");
}

TEST(ParseExpression, Comparisons) {
  EXPECT_EQ(parse_expr("a < b"), "(a < b)");
  EXPECT_EQ(parse_expr("a <= b"), "(a <= b)");
  EXPECT_EQ(parse_expr("a > b"), "(a > b)");
  EXPECT_EQ(parse_expr("a >= b"), "(a >= b)");
  EXPECT_EQ(parse_expr("a == b"), "(a == b)");
  EXPECT_EQ(parse_expr("a != b"), "(a != b)");
}

TEST(ParseExpression, EqualityBelowRelational) {
  EXPECT_EQ(parse_expr("a < b == c > d"), "((a < b) == (c > d))");
  EXPECT_EQ(parse_expr("a + 1 < b * 2"), "((a + 1) < (b * 2))");
}

TEST(ParseExpression, ComparisonsLeftAssociative) {
  EXPECT_EQ(parse_expr("a == b == c"), "((a == b) == c)");
  EXPECT_EQ(parse_expr("a < b < c"), "((a < b) < c)");
}

TEST(ParseExpression, Modulo) {
  EXPECT_EQ(parse_expr("a % b"), "(a % b)");
  EXPECT_EQ(parse_expr("a % b * c"), "((a % b) * c)");
  EXPECT_EQ(parse_expr("a + b % c"), "(a + (b % c))");
}

TEST(ParseExpression, UnaryInsideBinary) {
  EXPECT_EQ(parse_expr("a - -b"), "(a - (-b))");
  EXPECT_EQ(parse_expr("a * -b"), "(a * (-b))");
  EXPECT_EQ(parse_expr("--x"), "(-(-x))");
  EXPECT_EQ(parse_expr("-f(x)"), "(-f(x))");
  EXPECT_EQ(parse_expr("!(a && b)"), "(!(a && b))");
  EXPECT_EQ(parse_expr("!a && b"), "((!a) && b)");
}

TEST(ParseExpression, ParenthesesOverridePrecedence) {
  EXPECT_EQ(parse_expr("a * (b + c)"), "(a * (b + c))");
  EXPECT_EQ(parse_expr("(a || b) && c"), "((a || b) && c)");
  EXPECT_EQ(parse_expr("a - (b - c)"), "(a - (b - c))");
}

TEST(ParseExpression, CallsInExpressions) {
  EXPECT_EQ(parse_expr("f(a < b, !c)"), "f((a < b), (!c))");
  EXPECT_EQ(parse_expr("f(1) + g(2) * h()"), "(f(1) + (g(2) * h()))");
  EXPECT_EQ(parse_expr("f(g(h(x)))"), "f(g(h(x)))");
}

TEST(ParseExpression, NoSpaces) {
  EXPECT_EQ(parse_expr("1+2*3"), "(1 + (2 * 3))");
  EXPECT_EQ(parse_expr("a<=b&&c!=d"), "((a <= b) && (c != d))");
}

TEST(ParseExpression, LargeInteger) {
  EXPECT_EQ(parse_expr("9223372036854775807"), "9223372036854775807");
}

// ---------- Statements and functions ----------

namespace {
parser::program parse_program(const std::string &source) {
  std::vector<lexer::token> tokens;
  EXPECT_TRUE(lexer::tokenize(source, tokens));
  return parser::parse_to_ast(tokens);
}
} // namespace

TEST(ParseStatement, LetNameAndLocation) {
  auto prog = parse_program("fn main() {\n  let total = 1 + 2\n}");
  const auto &let = std::get<parser::let_stmt>(prog.functions.at(0).body.at(0));
  EXPECT_EQ(let.var.name, "total");
  EXPECT_EQ(let.var.location.row, 2u);
  EXPECT_EQ(let.var.location.col, 7u);
  EXPECT_EQ(dump(*let.expression), "(1 + 2)");
}

TEST(ParseStatement, MultipleStatements) {
  auto prog = parse_program("fn main() {\n"
                            "  let x = 1\n"
                            "  let y = x * 2\n"
                            "  x = x + y\n"
                            "  print(x, y)\n"
                            "}");
  const auto &body = prog.functions.at(0).body;
  ASSERT_EQ(body.size(), 4u);
  EXPECT_TRUE(std::holds_alternative<parser::let_stmt>(body[0]));
  EXPECT_TRUE(std::holds_alternative<parser::let_stmt>(body[1]));
  EXPECT_TRUE(std::holds_alternative<parser::assign_stmt>(body[2]));
  EXPECT_TRUE(std::holds_alternative<parser::call_stmt>(body[3]));
}

TEST(ParseStatement, Assignment) {
  auto prog = parse_program("fn main() {\n  x = x + 1\n}");
  const auto &a = std::get<parser::assign_stmt>(prog.functions.at(0).body.at(0));
  EXPECT_EQ(a.name, "x");
  EXPECT_EQ(a.location.row, 2u);
  EXPECT_EQ(a.location.col, 3u);
  EXPECT_EQ(dump(*a.expression), "(x + 1)");
}

TEST(ParseStatement, CallStatement) {
  auto prog = parse_program("fn main() {\n  f(1, g(2))\n}");
  const auto &c = std::get<parser::call_stmt>(prog.functions.at(0).body.at(0));
  EXPECT_EQ(c.name, "f");
  EXPECT_EQ(c.expression->_kind, kind::Call);
  EXPECT_EQ(dump(*c.expression), "f(1, g(2))");
}

TEST(ParseStatement, CallStatementNoArgs) {
  auto prog = parse_program("fn main() {\n  tick()\n}");
  const auto &c = std::get<parser::call_stmt>(prog.functions.at(0).body.at(0));
  EXPECT_EQ(dump(*c.expression), "tick()");
}

TEST(ParseStatement, IfOnly) {
  auto prog = parse_program("fn main() {\n"
                            "  if (x >= b) {\n"
                            "    x = x + 5\n"
                            "  }\n"
                            "}");
  const auto &s = std::get<parser::if_stmt>(prog.functions.at(0).body.at(0));
  ASSERT_EQ(s.branches.size(), 1u);
  EXPECT_EQ(dump(*s.branches[0].condition), "(x >= b)");
  ASSERT_EQ(s.branches[0].body.size(), 1u);
  EXPECT_TRUE(std::holds_alternative<parser::assign_stmt>(s.branches[0].body[0]));
  EXPECT_FALSE(s.else_body.has_value());
}

TEST(ParseStatement, IfElifElse) {
  auto prog = parse_program("fn main() {\n"
                            "  if (a) {\n"
                            "    x = 1\n"
                            "  } elif (b) {\n"
                            "    x = 2\n"
                            "  } elif (c) {\n"
                            "    x = 3\n"
                            "  } else {\n"
                            "    x = 4\n"
                            "  }\n"
                            "}");
  const auto &s = std::get<parser::if_stmt>(prog.functions.at(0).body.at(0));
  ASSERT_EQ(s.branches.size(), 3u);
  EXPECT_EQ(dump(*s.branches[0].condition), "a");
  EXPECT_EQ(dump(*s.branches[1].condition), "b");
  EXPECT_EQ(dump(*s.branches[2].condition), "c");
  ASSERT_TRUE(s.else_body.has_value());
  EXPECT_EQ(s.else_body->size(), 1u);
}

TEST(ParseStatement, NestedIf) {
  auto prog = parse_program("fn main() {\n"
                            "  if (a) {\n"
                            "    if (b) {\n"
                            "      x = 1\n"
                            "    }\n"
                            "  }\n"
                            "}");
  const auto &outer = std::get<parser::if_stmt>(prog.functions.at(0).body.at(0));
  ASSERT_EQ(outer.branches.at(0).body.size(), 1u);
  EXPECT_TRUE(
      std::holds_alternative<parser::if_stmt>(outer.branches[0].body[0]));
}

TEST(ParseStatement, IfFollowedByStatement) {
  auto prog = parse_program("fn main() {\n"
                            "  if (a) {\n"
                            "    x = 1\n"
                            "  }\n"
                            "  let y = 2\n"
                            "}");
  const auto &body = prog.functions.at(0).body;
  ASSERT_EQ(body.size(), 2u);
  EXPECT_TRUE(std::holds_alternative<parser::if_stmt>(body[0]));
  EXPECT_TRUE(std::holds_alternative<parser::let_stmt>(body[1]));
}

TEST(ParseFunction, EmptyBody) {
  auto prog = parse_program("fn main() {}");
  ASSERT_EQ(prog.functions.size(), 1u);
  EXPECT_EQ(prog.functions[0].name, "main");
  EXPECT_TRUE(prog.functions[0].parameters.empty());
  EXPECT_TRUE(prog.functions[0].body.empty());
  EXPECT_FALSE(prog.functions[0].is_spell);
}

TEST(ParseFunction, Parameters) {
  auto prog = parse_program("fn add(int x, int y) {\n  let z = x + y\n}");
  const auto &f = prog.functions.at(0);
  EXPECT_EQ(f.name, "add");
  ASSERT_EQ(f.parameters.size(), 2u);
  EXPECT_EQ(f.parameters[0].name, "x");
  EXPECT_EQ(f.parameters[1].name, "y");
  EXPECT_EQ(f.parameters[0]._type, parser::type::int_32);
}

TEST(ParseFunction, SpellFlag) {
  auto prog = parse_program("spell build() {\n  let x = 1\n}");
  ASSERT_EQ(prog.functions.size(), 1u);
  EXPECT_TRUE(prog.functions[0].is_spell);
  EXPECT_EQ(prog.functions[0].name, "build");
}

TEST(ParseFunction, MultipleFunctions) {
  auto prog = parse_program("fn a() {\n  let x = 1\n}\n"
                            "spell b() {\n  let y = 2\n}\n"
                            "fn main() {\n  a()\n}\n");
  ASSERT_EQ(prog.functions.size(), 3u);
  EXPECT_EQ(prog.functions[0].name, "a");
  EXPECT_EQ(prog.functions[1].name, "b");
  EXPECT_TRUE(prog.functions[1].is_spell);
  EXPECT_EQ(prog.functions[2].name, "main");
}

TEST(ParseFunction, Location) {
  auto prog = parse_program("\n\nfn foo() {}");
  EXPECT_EQ(prog.functions.at(0).location.row, 3u);
  EXPECT_EQ(prog.functions.at(0).location.col, 4u);
}

// ---------- Errors (parser calls std::terminate) ----------

TEST(ParseErrorDeathTest, TopLevelMustBeFunction) {
  EXPECT_DEATH(parse_program("let x = 1"), "expected 'fn' or 'spell'");
}

TEST(ParseErrorDeathTest, LetMissingEqual) {
  EXPECT_DEATH(parse_program("fn main() {\n  let x 1\n}"), "expected 'EQUAL'");
}

TEST(ParseErrorDeathTest, BareIdentifierStatement) {
  EXPECT_DEATH(parse_program("fn main() {\n  x\n}"),
               "expected '=' or '\\(' after identifier");
}

TEST(ParseErrorDeathTest, MissingOperand) {
  EXPECT_DEATH(parse_program("fn main() {\n  let x = 1 +\n}"),
               "expected expression");
}

TEST(ParseErrorDeathTest, UnsupportedParamType) {
  EXPECT_DEATH(parse_program("fn f(string s) {}"), "unsupported type");
}

TEST(ParseErrorDeathTest, MissingParamName) {
  EXPECT_DEATH(parse_program("fn f(int) {}"), "expected 'IDENTIFIER'");
}
