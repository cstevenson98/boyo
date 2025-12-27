#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "statement/expression.hpp"

namespace boyo {
namespace {

TEST(HexLiteralExpressionTest, ParseSingleByte) {
  HexLiteralExpression expr("0x10");
  EXPECT_EQ(expr.ToString(), "0x10");
  EXPECT_EQ(expr.GetHexString(), "0x10");
}

TEST(HexLiteralExpressionTest, ParseMultipleBytes) {
  HexLiteralExpression expr("0x1234");
  EXPECT_EQ(expr.ToString(), "0x1234");
  EXPECT_EQ(expr.GetHexString(), "0x1234");
}

TEST(HexLiteralExpressionTest, ParseOddLength) {
  HexLiteralExpression expr("0x123");
  EXPECT_EQ(expr.GetHexString(), "0x123");
}

TEST(HexLiteralExpressionTest, ParseLowercaseHex) {
  HexLiteralExpression expr("0xabcd");
  EXPECT_EQ(expr.GetHexString(), "0xabcd");
}

TEST(HexLiteralExpressionTest, ParseMixedCase) {
  HexLiteralExpression expr("0xAbCd");
  EXPECT_EQ(expr.GetHexString(), "0xAbCd");
}

TEST(HexLiteralExpressionTest, ParseZero) {
  HexLiteralExpression expr("0x0");
  EXPECT_EQ(expr.GetHexString(), "0x0");
}

TEST(HexLiteralExpressionTest, InvalidHexThrows) {
  // Missing "0x" prefix
  EXPECT_THROW(HexLiteralExpression("1234"), std::runtime_error);

  // Too short
  EXPECT_THROW(HexLiteralExpression("0"), std::runtime_error);
  EXPECT_THROW(HexLiteralExpression("x"), std::runtime_error);
  EXPECT_THROW(HexLiteralExpression("0x"), std::runtime_error);
}

// Test IdentifierExpression

TEST(IdentifierExpressionTest, CreatesAndReturnsName) {
  IdentifierExpression expr("double");
  EXPECT_EQ(expr.ToString(), "double");
  EXPECT_EQ(expr.GetName(), "double");
}

TEST(IdentifierExpressionTest, VariousIdentifiers) {
  IdentifierExpression expr1("A");
  EXPECT_EQ(expr1.GetName(), "A");

  IdentifierExpression expr2("myVariable");
  EXPECT_EQ(expr2.GetName(), "myVariable");

  IdentifierExpression expr3("func123");
  EXPECT_EQ(expr3.GetName(), "func123");
}

// Test ParameterExpression

TEST(ParameterExpressionTest, CreatesAndReturnsParamName) {
  ParameterExpression expr("_a");
  EXPECT_EQ(expr.ToString(), "_a");
  EXPECT_EQ(expr.GetParamName(), "_a");
}

TEST(ParameterExpressionTest, VariousParameters) {
  ParameterExpression expr1("_x");
  EXPECT_EQ(expr1.GetParamName(), "_x");

  ParameterExpression expr2("_param");
  EXPECT_EQ(expr2.GetParamName(), "_param");

  ParameterExpression expr3("_123");
  EXPECT_EQ(expr3.GetParamName(), "_123");
}

// Test OperatorExpression

TEST(OperatorExpressionTest, SimpleOperator) {
  // * 0x10 _a
  auto left = std::make_unique<HexLiteralExpression>("0x10");
  auto right = std::make_unique<ParameterExpression>("_a");

  OperatorExpression expr("*", std::move(left), std::move(right));

  EXPECT_EQ(expr.GetOperator(), "*");
  EXPECT_EQ(expr.ToString(), "* 0x10 _a");
}

TEST(OperatorExpressionTest, NestedOperatorLeft) {
  // * (+ 0x01 0x02) _a  ==  * + 0x01 0x02 _a
  auto inner_left = std::make_unique<HexLiteralExpression>("0x01");
  auto inner_right = std::make_unique<HexLiteralExpression>("0x02");
  auto nested = std::make_unique<OperatorExpression>("+", std::move(inner_left),
                                                     std::move(inner_right));

  auto param = std::make_unique<ParameterExpression>("_a");

  OperatorExpression expr("*", std::move(nested), std::move(param));

  EXPECT_EQ(expr.ToString(), "* + 0x01 0x02 _a");
}

TEST(OperatorExpressionTest, NestedOperatorRight) {
  // + _a (- _b 0x05)  ==  + _a - _b 0x05
  auto param_a = std::make_unique<ParameterExpression>("_a");

  auto param_b = std::make_unique<ParameterExpression>("_b");
  auto hex = std::make_unique<HexLiteralExpression>("0x05");
  auto nested = std::make_unique<OperatorExpression>("-", std::move(param_b),
                                                     std::move(hex));

  OperatorExpression expr("+", std::move(param_a), std::move(nested));

  EXPECT_EQ(expr.ToString(), "+ _a - _b 0x05");
}

TEST(OperatorExpressionTest, DeeplyNestedOperators) {
  // + (* 0x10 _a) (- _b 0x05)  ==  + * 0x10 _a - _b 0x05
  auto left_hex = std::make_unique<HexLiteralExpression>("0x10");
  auto left_param = std::make_unique<ParameterExpression>("_a");
  auto left_op = std::make_unique<OperatorExpression>("*", std::move(left_hex),
                                                      std::move(left_param));

  auto right_param = std::make_unique<ParameterExpression>("_b");
  auto right_hex = std::make_unique<HexLiteralExpression>("0x05");
  auto right_op = std::make_unique<OperatorExpression>(
      "-", std::move(right_param), std::move(right_hex));

  OperatorExpression expr("+", std::move(left_op), std::move(right_op));

  EXPECT_EQ(expr.ToString(), "+ * 0x10 _a - _b 0x05");
}

TEST(OperatorExpressionTest, VariousOperators) {
  auto left = std::make_unique<IdentifierExpression>("A");
  auto right = std::make_unique<IdentifierExpression>("B");

  OperatorExpression plus_expr("+", std::move(left), std::move(right));
  EXPECT_EQ(plus_expr.GetOperator(), "+");

  left = std::make_unique<IdentifierExpression>("A");
  right = std::make_unique<IdentifierExpression>("B");
  OperatorExpression minus_expr("-", std::move(left), std::move(right));
  EXPECT_EQ(minus_expr.GetOperator(), "-");
}

// Test KeywordExpression

TEST(KeywordExpressionTest, CreatesAndReturnsKeyword) {
  KeywordExpression expr("let");
  EXPECT_EQ(expr.ToString(), "let");
  EXPECT_EQ(expr.GetKeyword(), "let");
}

TEST(KeywordExpressionTest, VariousKeywords) {
  KeywordExpression expr1("def");
  EXPECT_EQ(expr1.GetKeyword(), "def");

  KeywordExpression expr2("main");
  EXPECT_EQ(expr2.GetKeyword(), "main");

  KeywordExpression expr3("print");
  EXPECT_EQ(expr3.GetKeyword(), "print");
}

// Test CreateExpression (token-based factory)

TEST(CreateExpressionTest, CreatesHexLiteralExpression) {
  Lexer::Token token{Lexer::TokenType::HEX_LITERAL, "0x10", 0, 0};
  auto expr = CreateExpression(token);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "0x10");

  auto *hex_expr = dynamic_cast<HexLiteralExpression *>(expr.get());
  ASSERT_NE(hex_expr, nullptr);
  EXPECT_EQ(hex_expr->GetHexString(), "0x10");
}

TEST(CreateExpressionTest, CreatesIdentifierExpression) {
  Lexer::Token token{Lexer::TokenType::IDENTIFIER, "double", 0, 0};
  auto expr = CreateExpression(token);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "double");

  auto *id_expr = dynamic_cast<IdentifierExpression *>(expr.get());
  ASSERT_NE(id_expr, nullptr);
  EXPECT_EQ(id_expr->GetName(), "double");
}

TEST(CreateExpressionTest, CreatesParameterExpression) {
  Lexer::Token token{Lexer::TokenType::PARAM_IDENTIFIER, "_a", 0, 0};
  auto expr = CreateExpression(token);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "_a");

  auto *param_expr = dynamic_cast<ParameterExpression *>(expr.get());
  ASSERT_NE(param_expr, nullptr);
  EXPECT_EQ(param_expr->GetParamName(), "_a");
}

TEST(CreateExpressionTest, CreatesKeywordExpression) {
  Lexer::Token token{Lexer::TokenType::KEYWORD_LET, "let", 0, 0};
  auto expr = CreateExpression(token);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "let");

  auto *kw_expr = dynamic_cast<KeywordExpression *>(expr.get());
  ASSERT_NE(kw_expr, nullptr);
  EXPECT_EQ(kw_expr->GetKeyword(), "let");
}

TEST(CreateExpressionTest, ThrowsForOperatorTokens) {
  Lexer::Token token{Lexer::TokenType::OPERATOR_PLUS, "+", 0, 0};
  EXPECT_THROW(CreateExpression(token), std::runtime_error);
}

TEST(CreateExpressionTest, ThrowsForSymbolTokens) {
  Lexer::Token equals{Lexer::TokenType::EQUALS, "=", 0, 0};
  EXPECT_THROW(CreateExpression(equals), std::runtime_error);

  Lexer::Token arrow{Lexer::TokenType::ARROW, "=>", 0, 0};
  EXPECT_THROW(CreateExpression(arrow), std::runtime_error);
}

// Test ParsePolishExpression

TEST(ParsePolishExpressionTest, ParsesSimpleLeafExpression) {
  Lexer::TokenList tokens = {{Lexer::TokenType::HEX_LITERAL, "0x10", 0, 0}};

  size_t index = 0;
  auto expr = ParsePolishExpression(tokens, index);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "0x10");
  EXPECT_EQ(index, 1); // Token consumed

  auto *hex_expr = dynamic_cast<HexLiteralExpression *>(expr.get());
  ASSERT_NE(hex_expr, nullptr);
}

TEST(ParsePolishExpressionTest, ParsesSimpleOperator) {
  // * 0x10 _a
  Lexer::TokenList tokens = {{Lexer::TokenType::OPERATOR_MULTIPLY, "*", 0, 0},
                             {Lexer::TokenType::HEX_LITERAL, "0x10", 0, 2},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_a", 0, 7}};

  size_t index = 0;
  auto expr = ParsePolishExpression(tokens, index);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "* 0x10 _a");
  EXPECT_EQ(index, 3); // All tokens consumed

  auto *op_expr = dynamic_cast<OperatorExpression *>(expr.get());
  ASSERT_NE(op_expr, nullptr);
  EXPECT_EQ(op_expr->GetOperator(), "*");
}

TEST(ParsePolishExpressionTest, ParsesNestedOperatorLeft) {
  // * + 0x01 0x02 _a  ==  * (+ 0x01 0x02) _a
  Lexer::TokenList tokens = {{Lexer::TokenType::OPERATOR_MULTIPLY, "*", 0, 0},
                             {Lexer::TokenType::OPERATOR_PLUS, "+", 0, 2},
                             {Lexer::TokenType::HEX_LITERAL, "0x01", 0, 4},
                             {Lexer::TokenType::HEX_LITERAL, "0x02", 0, 9},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_a", 0, 14}};

  size_t index = 0;
  auto expr = ParsePolishExpression(tokens, index);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "* + 0x01 0x02 _a");
  EXPECT_EQ(index, 5);

  auto *op_expr = dynamic_cast<OperatorExpression *>(expr.get());
  ASSERT_NE(op_expr, nullptr);
  EXPECT_EQ(op_expr->GetOperator(), "*");

  // Check left is nested operator
  auto *left_op = dynamic_cast<const OperatorExpression *>(&op_expr->GetLeft());
  ASSERT_NE(left_op, nullptr);
  EXPECT_EQ(left_op->GetOperator(), "+");
}

TEST(ParsePolishExpressionTest, ParsesNestedOperatorRight) {
  // + _a - _b 0x05  ==  + _a (- _b 0x05)
  Lexer::TokenList tokens = {{Lexer::TokenType::OPERATOR_PLUS, "+", 0, 0},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_a", 0, 2},
                             {Lexer::TokenType::OPERATOR_MINUS, "-", 0, 5},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_b", 0, 7},
                             {Lexer::TokenType::HEX_LITERAL, "0x05", 0, 10}};

  size_t index = 0;
  auto expr = ParsePolishExpression(tokens, index);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "+ _a - _b 0x05");
  EXPECT_EQ(index, 5);

  auto *op_expr = dynamic_cast<OperatorExpression *>(expr.get());
  ASSERT_NE(op_expr, nullptr);

  // Check right is nested operator
  auto *right_op =
      dynamic_cast<const OperatorExpression *>(&op_expr->GetRight());
  ASSERT_NE(right_op, nullptr);
  EXPECT_EQ(right_op->GetOperator(), "-");
}

TEST(ParsePolishExpressionTest, ParsesDeeplyNestedOperators) {
  // + * 0x10 _a - _b 0x05
  Lexer::TokenList tokens = {{Lexer::TokenType::OPERATOR_PLUS, "+", 0, 0},
                             {Lexer::TokenType::OPERATOR_MULTIPLY, "*", 0, 2},
                             {Lexer::TokenType::HEX_LITERAL, "0x10", 0, 4},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_a", 0, 9},
                             {Lexer::TokenType::OPERATOR_MINUS, "-", 0, 12},
                             {Lexer::TokenType::PARAM_IDENTIFIER, "_b", 0, 14},
                             {Lexer::TokenType::HEX_LITERAL, "0x05", 0, 17}};

  size_t index = 0;
  auto expr = ParsePolishExpression(tokens, index);

  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "+ * 0x10 _a - _b 0x05");
  EXPECT_EQ(index, 7);

  auto *op_expr = dynamic_cast<OperatorExpression *>(expr.get());
  ASSERT_NE(op_expr, nullptr);
  EXPECT_EQ(op_expr->GetOperator(), "+");

  // Both left and right should be operators
  auto *left_op = dynamic_cast<const OperatorExpression *>(&op_expr->GetLeft());
  auto *right_op =
      dynamic_cast<const OperatorExpression *>(&op_expr->GetRight());
  ASSERT_NE(left_op, nullptr);
  ASSERT_NE(right_op, nullptr);
}

TEST(ParsePolishExpressionTest, ThrowsOnEmptyTokenList) {
  Lexer::TokenList tokens;
  size_t index = 0;
  EXPECT_THROW(ParsePolishExpression(tokens, index), std::runtime_error);
}

TEST(ParsePolishExpressionTest, ThrowsOnIndexOutOfBounds) {
  Lexer::TokenList tokens = {{Lexer::TokenType::HEX_LITERAL, "0x10", 0, 0}};
  size_t index = 10; // Beyond tokens size
  EXPECT_THROW(ParsePolishExpression(tokens, index), std::runtime_error);
}

} // namespace
} // namespace boyo
