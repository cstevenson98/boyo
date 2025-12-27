#include <iostream>
#include <vector>

#include "compiler/compiler.hpp"
#include "parser/parser.hpp"
#include "utils/code_printer.hpp"

using namespace boyo;

int main() {
  std::cout << "==================================================\n";
  std::cout << "       Boyo Compiler - Complete Pipeline Demo\n";
  std::cout << "==================================================\n\n";

  // Example Boyo program
  std::vector<std::string> boyo_program = {
      "// Example: Double a value", "let A 0x10", "def double _a => * 0x02 _a",
      "main double A"};

  std::cout << "Boyo Source Code:\n";
  std::cout << "-----------------\n";
  for (const auto &line : boyo_program) {
    std::cout << "  " << line << "\n";
  }
  std::cout << "\n";

  // Parse into statements
  std::cout << "Step 1: Lexing and Parsing...\n";
  Parser parser;
  auto statements = parser.Parse(boyo_program);
  std::cout << "✓ Parsed " << statements.size() << " statements\n\n";

  // Generate C++ code
  std::cout << "Step 2: Generating C++ Code...\n";
  auto generated_code = Compiler::GenerateProgramCode(statements);
  std::cout << "✓ Generated " << generated_code.size() << " bytes of code\n\n";

  // Pretty print the generated code
  CodePrinter printer;
  printer.Print(generated_code);

  std::cout << "\n==================================================\n";
  std::cout << "Phase 4 Complete: Full Boyo → C++ Pipeline Working!\n";
  std::cout << "==================================================\n";

  return 0;
}
