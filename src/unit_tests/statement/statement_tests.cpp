#include <gtest/gtest.h>

#include <memory>

#include "lexer/lexer.hpp"
#include "statement/expression.hpp"
#include "statement/statement.hpp"

namespace boyo {
namespace {

/**
 * LetStatement Tests
 */

TEST(LetStatementTest, Constructor_StoresVarNameAndExpression) {
  auto value_expr = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement let_stmt("A", std::move(value_expr));

  EXPECT_EQ(let_stmt.GetVarName(), "A");
  EXPECT_EQ(let_stmt.GetValueExpr().ToString(), "0x10");
}

TEST(LetStatementTest, GenerateCode_SimpleHexLiteral) {
  auto value_expr = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement let_stmt("A", std::move(value_expr));

  std::string code = let_stmt.GenerateCode();
  EXPECT_EQ(code, "std::vector<uint8_t> A = {0x10};\n");
}

TEST(LetStatementTest, GenerateCode_MultiByteHex) {
  auto value_expr = std::make_unique<HexLiteralExpression>("0xDEADBEEF");
  LetStatement let_stmt("result", std::move(value_expr));

  std::string code = let_stmt.GenerateCode();
  EXPECT_EQ(code, "std::vector<uint8_t> result = {0xDEADBEEF};\n");
}

/**
 * DefStatement Tests
 */

TEST(DefStatementTest, Constructor_StoresFuncNameParamsAndBody) {
  auto body_expr = std::make_unique<ParameterExpression>("_a");
  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("identity", params, std::move(body_expr));

  EXPECT_EQ(def_stmt.GetFuncName(), "identity");
  EXPECT_EQ(def_stmt.GetParams().size(), 1);
  EXPECT_EQ(def_stmt.GetParams()[0], "_a");
  EXPECT_EQ(def_stmt.GetBodyExpr().ToString(), "_a");
}

TEST(DefStatementTest, GenerateCode_IdentityFunction) {
  auto body_expr = std::make_unique<ParameterExpression>("_a");
  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("identity", params, std::move(body_expr));

  std::string code = def_stmt.GenerateCode();
  EXPECT_EQ(code,
            "std::vector<uint8_t> identity(const std::vector<uint8_t>& _a) {\n"
            "  return _a;\n"
            "}\n");
}

TEST(DefStatementTest, GenerateCode_MultiplyByConstant) {
  // def double _a => * 0x10 _a
  auto left = std::make_unique<HexLiteralExpression>("0x10");
  auto right = std::make_unique<ParameterExpression>("_a");
  auto body_expr = std::make_unique<OperatorExpression>("*", std::move(left),
                                                        std::move(right));

  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("double", params, std::move(body_expr));

  std::string code = def_stmt.GenerateCode();
  EXPECT_EQ(code,
            "std::vector<uint8_t> double(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors({0x10}, _a);\n"
            "}\n");
}

TEST(DefStatementTest, GenerateCode_NestedOperators) {
  // def calc _a => * + 0x01 0x02 _a
  auto add_left = std::make_unique<HexLiteralExpression>("0x01");
  auto add_right = std::make_unique<HexLiteralExpression>("0x02");
  auto add_expr = std::make_unique<OperatorExpression>("+", std::move(add_left),
                                                       std::move(add_right));

  auto param_expr = std::make_unique<ParameterExpression>("_a");
  auto body_expr = std::make_unique<OperatorExpression>(
      "*", std::move(add_expr), std::move(param_expr));

  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("calc", params, std::move(body_expr));

  std::string code = def_stmt.GenerateCode();
  EXPECT_EQ(code,
            "std::vector<uint8_t> calc(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors(add_vectors({0x01}, {0x02}), _a);\n"
            "}\n");
}

TEST(DefStatementTest, GenerateCode_MultipleParameters) {
  // def add _a _b => + _a _b
  auto left = std::make_unique<ParameterExpression>("_a");
  auto right = std::make_unique<ParameterExpression>("_b");
  auto body_expr = std::make_unique<OperatorExpression>("+", std::move(left),
                                                        std::move(right));

  std::vector<std::string> params = {"_a", "_b"};
  DefStatement def_stmt("add", params, std::move(body_expr));

  std::string code = def_stmt.GenerateCode();
  EXPECT_EQ(code,
            "std::vector<uint8_t> add(const std::vector<uint8_t>& _a, const "
            "std::vector<uint8_t>& _b) {\n"
            "  return add_vectors(_a, _b);\n"
            "}\n");
}

/**
 * MainStatement Tests
 */

TEST(MainStatementTest, Constructor_StoresFuncNameAndArgs) {
  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("double", args);

  EXPECT_EQ(main_stmt.GetFuncName(), "double");
  EXPECT_EQ(main_stmt.GetArgs().size(), 1);
  EXPECT_EQ(main_stmt.GetArgs()[0], "A");
}

TEST(MainStatementTest, GenerateCode_SingleArg) {
  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("double", args);

  std::string code = main_stmt.GenerateCode();
  EXPECT_EQ(code, "auto result = double(A);\n"
                  "print_vector(std::cout, result);\n");
}

TEST(MainStatementTest, GenerateCode_MultipleArgs) {
  std::vector<std::string> args = {"A", "B", "C"};
  MainStatement main_stmt("compute", args);

  std::string code = main_stmt.GenerateCode();
  EXPECT_EQ(code, "auto result = compute(A, B, C);\n"
                  "print_vector(std::cout, result);\n");
}

TEST(MainStatementTest, GenerateCode_NoArgs) {
  std::vector<std::string> args = {};
  MainStatement main_stmt("get_value", args);

  std::string code = main_stmt.GenerateCode();
  EXPECT_EQ(code, "auto result = get_value();\n"
                  "print_vector(std::cout, result);\n");
}

/**
 * Integration Tests - Multiple Statements Together
 */

TEST(StatementIntegrationTest, LetAndMain) {
  // let A 0x10
  auto let_value = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement let_stmt("A", std::move(let_value));

  // main identity A
  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("identity", args);

  std::string combined_code =
      let_stmt.GenerateCode() + main_stmt.GenerateCode();
  EXPECT_EQ(combined_code, "std::vector<uint8_t> A = {0x10};\n"
                           "auto result = identity(A);\n"
                           "print_vector(std::cout, result);\n");
}

TEST(StatementIntegrationTest, DefAndMain) {
  // def double _a => * 0x10 _a
  auto left = std::make_unique<HexLiteralExpression>("0x10");
  auto right = std::make_unique<ParameterExpression>("_a");
  auto body_expr = std::make_unique<OperatorExpression>("*", std::move(left),
                                                        std::move(right));
  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("double", params, std::move(body_expr));

  // main double A
  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("double", args);

  std::string combined_code =
      def_stmt.GenerateCode() + main_stmt.GenerateCode();
  EXPECT_EQ(combined_code,
            "std::vector<uint8_t> double(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors({0x10}, _a);\n"
            "}\n"
            "auto result = double(A);\n"
            "print_vector(std::cout, result);\n");
}

TEST(StatementIntegrationTest, CompleteProgram) {
  // let A 0x10
  auto let_value = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement let_stmt("A", std::move(let_value));

  // def double _a => * 0x10 _a
  auto mul_left = std::make_unique<HexLiteralExpression>("0x10");
  auto mul_right = std::make_unique<ParameterExpression>("_a");
  auto def_body = std::make_unique<OperatorExpression>("*", std::move(mul_left),
                                                       std::move(mul_right));
  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("double", params, std::move(def_body));

  // main double A
  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("double", args);

  std::string program_code = let_stmt.GenerateCode() + def_stmt.GenerateCode() +
                             main_stmt.GenerateCode();

  EXPECT_EQ(program_code,
            "std::vector<uint8_t> A = {0x10};\n"
            "std::vector<uint8_t> double(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors({0x10}, _a);\n"
            "}\n"
            "auto result = double(A);\n"
            "print_vector(std::cout, result);\n");
}

} // namespace
} // namespace boyo
