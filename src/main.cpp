#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cli.hpp"
#include "compiler/compiler.hpp"
#include "parser/parser.hpp"
#include "statement/statement.hpp"
#include "utils/code_printer.hpp"

int main(int argc, char *argv[]) {
  cli::CliExecutor executor("boyo", "Boyo compiler");

  // Set usage string
  executor.set_usage("<input.boyo> [-o <output>] [--print-code] [--print-ast]");

  // Add output flag
  executor.add_flag("-o,--output", cli::FlagType::MultiArg,
                    "Output file path (required unless --print-code/--print-ast is used)", false);

  // Add print-code flag
  executor.add_flag("--print-code", cli::FlagType::Boolean,
                    "Print generated C++ code without compiling", false);

  // Add print-ast flag
  executor.add_flag("--print-ast", cli::FlagType::Boolean,
                    "Print Abstract Syntax Tree (AST) structure", false);

  // Set handler for command-less mode
  executor.set_handler([](const cli::ParseResult &result) {
    // Get input file (first positional argument)
    if (result.positional_args.empty()) {
      std::fprintf(stderr, "Error: No input file specified\n");
      return 1;
    }

    const std::string &input_file = result.positional_args[0];

    // Check if --print-code flag is set
    bool print_code = result.has_flag("--print-code");
    bool print_ast = result.has_flag("--print-ast");

    // Get output file (required unless --print-code or --print-ast is used)
    auto output_args = result.get_args("--output");
    if (!print_code && !print_ast && output_args.empty()) {
      std::fprintf(stderr,
                   "Error: Output file not specified (use -o or --output)\n");
      return 1;
    }

    // Read the input file
    std::ifstream in(input_file);
    if (!in) {
      std::fprintf(stderr, "Error: Failed to open input file: %s\n",
                   input_file.c_str());
      return 1;
    }

    // Read all lines from the input file
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
      lines.push_back(line);
    }
    in.close();

    try {
      if (print_ast) {
        // Parse and print AST structure
        boyo::Parser parser;
        auto statements = parser.Parse(lines);
        
        std::cout << "\n";
        std::cout << "=== Abstract Syntax Tree ===\n";
        std::cout << "Program (" << statements.size() << " statements)\n";
        
        for (size_t i = 0; i < statements.size(); ++i) {
          std::cout << "\n[" << (i + 1) << "] ";
          
          // Identify statement type and print structure
          if (auto* let_stmt = dynamic_cast<boyo::LetStatement*>(statements[i].get())) {
            std::cout << "LetStatement\n";
            std::cout << "  ├─ Variable: " << let_stmt->GetVarName() << "\n";
            std::cout << "  └─ Value: " << let_stmt->GetValueExpr().ToString() << "\n";
          } else if (auto* def_stmt = dynamic_cast<boyo::DefStatement*>(statements[i].get())) {
            std::cout << "DefStatement\n";
            std::cout << "  ├─ Function: " << def_stmt->GetFuncName() << "\n";
            std::cout << "  ├─ Parameters: [";
            const auto& params = def_stmt->GetParams();
            for (size_t j = 0; j < params.size(); ++j) {
              if (j > 0) std::cout << ", ";
              std::cout << params[j];
            }
            std::cout << "]\n";
            std::cout << "  └─ Body: " << def_stmt->GetBodyExpr().ToString() << "\n";
          } else if (auto* main_stmt = dynamic_cast<boyo::MainStatement*>(statements[i].get())) {
            std::cout << "MainStatement\n";
            std::cout << "  ├─ Function: " << main_stmt->GetFuncName() << "\n";
            std::cout << "  └─ Arguments: [";
            const auto& args = main_stmt->GetArgs();
            for (size_t j = 0; j < args.size(); ++j) {
              if (j > 0) std::cout << ", ";
              std::cout << args[j];
            }
            std::cout << "]\n";
          } else {
            std::cout << "Statement (comment or other)\n";
          }
        }
        
        std::cout << "\n";
        return 0;
      } else if (print_code) {
        // Parse and generate code without compiling
        boyo::Parser parser;
        auto statements = parser.Parse(lines);
        auto program_code = boyo::Compiler::GenerateProgramCode(statements);
        auto full_code = boyo::Compiler::SubstituteGeneratedCode(
            boyo::Compiler::GetMainFunctionSnippet(), program_code);

        // Print the generated code with formatting
        boyo::CodePrinter printer;
        printer.Print(full_code);
        return 0;
      } else {
        // Compile the program normally
        const std::string &output_file = output_args[0];
        boyo::Compiler compiler;
        compiler.compile(lines, output_file);
        std::printf("Successfully compiled %s -> %s\n", input_file.c_str(),
                    output_file.c_str());
        return 0;
      }
    } catch (const std::exception &e) {
      std::fprintf(stderr, "Error: %s\n", e.what());
      return 1;
    }
  });

  return executor.run(argc, argv);
}
