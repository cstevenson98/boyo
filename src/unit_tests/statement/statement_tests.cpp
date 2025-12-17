#include <gtest/gtest.h>

#include <memory>

#include "statement/expression.hpp"
#include "statement/statement.hpp"

namespace boyo {
namespace {

// Test for PrintStatement with expressions

TEST(PrintStatementTest, GenerateCode_PrintsLiteralExpression) {
  ExpressionList expressions;
  expressions.push_back(CreateExpression("print"));
  expressions.push_back(CreateExpression("Hello World"));
  
  PrintStatement print_statement(std::move(expressions));
  std::string code = print_statement.GenerateCode();
  EXPECT_EQ(code, "std::cout << \"Hello World\" << std::endl;\n");
}

TEST(PrintStatementTest, GetExpressions_ReturnsExpressions) {
  ExpressionList expressions;
  expressions.push_back(CreateExpression("print"));
  expressions.push_back(CreateExpression("test"));
  
  PrintStatement print_statement(std::move(expressions));
  const auto& exprs = print_statement.GetExpressions();
  EXPECT_EQ(exprs.size(), 2);
  EXPECT_EQ(exprs[0]->ToString(), "print");
  EXPECT_EQ(exprs[1]->ToString(), "test");
}

}  // namespace
}  // namespace boyo

