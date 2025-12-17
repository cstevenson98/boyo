#include <gtest/gtest.h>

#include "statement/expression.hpp"

namespace boyo {
namespace {

// Test for Expression factory function

TEST(ExpressionFactoryTest, CreateExpression_CreatesFunctionHeadForPrint) {
  auto expr = CreateExpression("print");
  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "print");
  
  auto* func_head = dynamic_cast<FunctionHeadExpression*>(expr.get());
  ASSERT_NE(func_head, nullptr);
  EXPECT_EQ(func_head->GetName(), "print");
}

TEST(ExpressionFactoryTest, CreateExpression_CreatesLiteralForNonPrint) {
  auto expr = CreateExpression("Hello");
  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->ToString(), "Hello");
  
  auto* literal = dynamic_cast<LiteralExpression*>(expr.get());
  ASSERT_NE(literal, nullptr);
  EXPECT_EQ(literal->GetValue(), "Hello");
}

TEST(ExpressionFactoryTest, CreateExpression_CreatesLiteralForVariousStrings) {
  auto expr1 = CreateExpression("World");
  auto* literal1 = dynamic_cast<LiteralExpression*>(expr1.get());
  ASSERT_NE(literal1, nullptr);
  EXPECT_EQ(literal1->GetValue(), "World");

  auto expr2 = CreateExpression("123");
  auto* literal2 = dynamic_cast<LiteralExpression*>(expr2.get());
  ASSERT_NE(literal2, nullptr);
  EXPECT_EQ(literal2->GetValue(), "123");

  auto expr3 = CreateExpression("test_string");
  auto* literal3 = dynamic_cast<LiteralExpression*>(expr3.get());
  ASSERT_NE(literal3, nullptr);
  EXPECT_EQ(literal3->GetValue(), "test_string");
}

// Test for FunctionHeadExpression

TEST(FunctionHeadExpressionTest, ToString_ReturnsName) {
  FunctionHeadExpression expr("print");
  EXPECT_EQ(expr.ToString(), "print");
  EXPECT_EQ(expr.GetName(), "print");
}

// Test for LiteralExpression

TEST(LiteralExpressionTest, ToString_ReturnsValue) {
  LiteralExpression expr("Hello World");
  EXPECT_EQ(expr.ToString(), "Hello World");
  EXPECT_EQ(expr.GetValue(), "Hello World");
}

}  // namespace
}  // namespace boyo

