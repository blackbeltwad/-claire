#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "lexer.h"

using eclaire::lexer::token;
using eclaire::lexer::token_type;
using eclaire::lexer::tokenize;

namespace {

std::vector<token> lex(const std::string &source) {
  std::vector<token> tokens;
  EXPECT_TRUE(tokenize(source, tokens));
  return tokens;
}

std::vector<token_type> types(const std::string &source) {
  std::vector<token_type> out;
  for (const auto &t : lex(source)) {
    out.push_back(t.type);
  }
  return out;
}

bool lex_fails(const std::string &source) {
  std::vector<token> tokens;
  return !tokenize(source, tokens);
}

} // namespace

TEST(Lexer, LetStatement) {
  std::vector<token_type> expected = {token_type::LET, token_type::IDENTIFIER,
                                      token_type::EQUAL, token_type::INTEGER};
  EXPECT_EQ(types("let x = 5"), expected);
}

TEST(Lexer, Keywords) {
  auto t = lex("fn spell invoke release if elif else while");
  ASSERT_EQ(t.size(), 8u);
  EXPECT_EQ(t[0].type, token_type::FN);
  EXPECT_EQ(t[1].type, token_type::SPELL);
  EXPECT_EQ(t[2].type, token_type::INVOKE);
  EXPECT_EQ(t[3].type, token_type::RELEASE);
  EXPECT_EQ(t[4].type, token_type::IF);
  EXPECT_EQ(t[5].type, token_type::ELIF);
  EXPECT_EQ(t[6].type, token_type::ELSE);
  EXPECT_EQ(t[7].type, token_type::WHILE);
}

TEST(Lexer, TypesAreTypeTokens) {
  auto t = lex("int string");
  ASSERT_EQ(t.size(), 2u);
  EXPECT_EQ(t[0].type, token_type::TYPE);
  EXPECT_EQ(t[1].type, token_type::TYPE);
}

TEST(Lexer, IdentifierWithUnderscoreAndDigits) {
  auto t = lex("_foo_1");
  ASSERT_EQ(t.size(), 1u);
  EXPECT_EQ(t[0].type, token_type::IDENTIFIER);
  EXPECT_EQ(t[0].text, "_foo_1");
}

TEST(Lexer, EqualVsEqualEqual) {
  auto t = lex("a = b == c");
  ASSERT_EQ(t.size(), 5u);
  EXPECT_EQ(t[1].type, token_type::EQUAL);
  EXPECT_EQ(t[3].type, token_type::OPERATOR);
  EXPECT_EQ(t[3].text, "==");
}

TEST(Lexer, TwoCharOperators) {
  auto t = lex("a <= b >= c != d");
  ASSERT_EQ(t.size(), 7u);
  EXPECT_EQ(t[1].text, "<=");
  EXPECT_EQ(t[3].text, ">=");
  EXPECT_EQ(t[5].text, "!=");
}

TEST(Lexer, LogicalOperators) {
  // needs the && / || lexer fix
  auto t = lex("a && b || c");
  ASSERT_EQ(t.size(), 5u);
  EXPECT_EQ(t[1].text, "&&");
  EXPECT_EQ(t[3].text, "||");
}

TEST(Lexer, StringLiteral) {
  auto t = lex("`hello world`");
  ASSERT_EQ(t.size(), 3u);
  EXPECT_EQ(t[0].type, token_type::QUOTE);
  EXPECT_EQ(t[1].type, token_type::STRING);
  EXPECT_EQ(t[1].text, "hello world");
  EXPECT_EQ(t[2].type, token_type::QUOTE);
}

TEST(Lexer, NewlineTokens) {
  std::vector<token_type> expected = {
      token_type::IDENTIFIER, token_type::NEWLINE, token_type::IDENTIFIER};
  EXPECT_EQ(types("a\nb"), expected);
}

TEST(Lexer, Locations) {
  auto t = lex("let x\n  y");
  ASSERT_EQ(t.size(), 4u); // let, x, NEWLINE, y
  EXPECT_EQ(t[1].location.row, 1u);
  EXPECT_EQ(t[1].location.col, 5u);
  EXPECT_EQ(t[3].location.row, 2u);
  EXPECT_EQ(t[3].location.col, 3u);
}

TEST(Lexer, EmptyInput) { EXPECT_TRUE(lex("").empty()); }

TEST(Lexer, Errors) {
  EXPECT_TRUE(lex_fails("123abc"));        // invalid integer literal
  EXPECT_TRUE(lex_fails("@"));             // unexpected character
  EXPECT_TRUE(lex_fails("(a"));            // unclosed delimiter
  EXPECT_TRUE(lex_fails("a)"));            // mismatched closing delimiter
  EXPECT_TRUE(lex_fails("(a]"));           // wrong closing delimiter
  EXPECT_TRUE(lex_fails("`never closed")); // unterminated string
}
