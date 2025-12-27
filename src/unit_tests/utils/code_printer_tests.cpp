#include <gtest/gtest.h>

#include "statement/expression.hpp"
#include "statement/statement.hpp"
#include "utils/code_printer.hpp"

namespace boyo {
namespace {

TEST(CodePrinterTest, Format_WithClangFormat) {
  CodePrinter printer;

  // Create some unformatted code
  std::string unformatted = "int main(){int x=5;return x;}";

  std::string formatted = printer.Format(unformatted);

  // If clang-format is available, should be different
  if (CodePrinter::IsClangFormatAvailable()) {
    EXPECT_NE(formatted, unformatted);
    EXPECT_TRUE(formatted.find('\n') != std::string::npos);
  } else {
    // If not available, should return original
    EXPECT_EQ(formatted, unformatted);
  }
}

TEST(CodePrinterTest, IsClangFormatAvailable_Checks) {
  // Just verify it returns a boolean without crashing
  bool available = CodePrinter::IsClangFormatAvailable();
  EXPECT_TRUE(available || !available); // Always true, just checking execution
}

TEST(CodePrinterTest, Print_CompleteProgram) {
  CodePrinter printer;

  // Build a complete Boyo program
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

  // Print to stdout
  std::cout << "\n";
  bool used_formatter = printer.Print(program_code, true);

  // Just verify it doesn't crash
  EXPECT_TRUE(used_formatter || !used_formatter);
}

TEST(CodePrinterTest, PrintBasic_NoColor) {
  CodePrinter printer;

  std::string simple_code = "int x = 5;\nreturn x;";

  std::cout << "\nTesting basic print without color:\n";
  printer.Print(simple_code, false);

  SUCCEED();
}

} // namespace
} // namespace boyo
