#include <iostream>

#include "statement/expression.hpp"
#include "statement/statement.hpp"
#include "utils/code_printer.hpp"

using namespace boyo;

int main() {
  CodePrinter printer;

  std::cout << "===========================================\n";
  std::cout << "  Boyo Code Generator Demo\n";
  std::cout << "===========================================\n\n";

  // Example 1: Simple variable declaration
  std::cout << "Example 1: Variable Declaration\n";
  std::cout << "Boyo: let A 0x10\n\n";

  auto let_value = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement let_stmt("A", std::move(let_value));

  printer.Print(let_stmt.GenerateCode());

  // Example 2: Function definition
  std::cout << "\n\nExample 2: Function Definition\n";
  std::cout << "Boyo: def double _a => * 0x10 _a\n\n";

  auto mul_left = std::make_unique<HexLiteralExpression>("0x10");
  auto mul_right = std::make_unique<ParameterExpression>("_a");
  auto def_body = std::make_unique<OperatorExpression>("*", std::move(mul_left),
                                                       std::move(mul_right));
  std::vector<std::string> params = {"_a"};
  DefStatement def_stmt("double", params, std::move(def_body));

  printer.Print(def_stmt.GenerateCode());

  // Example 3: Main statement
  std::cout << "\n\nExample 3: Main Entry Point\n";
  std::cout << "Boyo: main double A\n\n";

  std::vector<std::string> args = {"A"};
  MainStatement main_stmt("double", args);

  printer.Print(main_stmt.GenerateCode());

  // Example 4: Complete program
  std::cout << "\n\nExample 4: Complete Program\n";
  std::cout << "Boyo Code:\n";
  std::cout << "  let A 0x10\n";
  std::cout << "  def double _a => * 0x10 _a\n";
  std::cout << "  main double A\n\n";

  auto complete_let_value = std::make_unique<HexLiteralExpression>("0x10");
  LetStatement complete_let("A", std::move(complete_let_value));

  auto complete_mul_left = std::make_unique<HexLiteralExpression>("0x10");
  auto complete_mul_right = std::make_unique<ParameterExpression>("_a");
  auto complete_def_body = std::make_unique<OperatorExpression>(
      "*", std::move(complete_mul_left), std::move(complete_mul_right));
  std::vector<std::string> complete_params = {"_a"};
  DefStatement complete_def("double", complete_params,
                            std::move(complete_def_body));

  std::vector<std::string> complete_args = {"A"};
  MainStatement complete_main("double", complete_args);

  std::string program = complete_let.GenerateCode() +
                        complete_def.GenerateCode() +
                        complete_main.GenerateCode();

  printer.Print(program);

  // Example 5: Nested operators
  std::cout << "\n\nExample 5: Nested Operator Expression\n";
  std::cout << "Boyo: def calc _a => * + 0x01 0x02 _a\n\n";

  auto add_left = std::make_unique<HexLiteralExpression>("0x01");
  auto add_right = std::make_unique<HexLiteralExpression>("0x02");
  auto add_expr = std::make_unique<OperatorExpression>("+", std::move(add_left),
                                                       std::move(add_right));

  auto param_expr = std::make_unique<ParameterExpression>("_a");
  auto nested_body = std::make_unique<OperatorExpression>(
      "*", std::move(add_expr), std::move(param_expr));

  std::vector<std::string> nested_params = {"_a"};
  DefStatement nested_def("calc", nested_params, std::move(nested_body));

  printer.Print(nested_def.GenerateCode());

  std::cout << "\n===========================================\n";

  return 0;
}
