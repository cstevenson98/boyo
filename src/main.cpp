#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "cli.hpp"
#include "compiler/compiler.hpp"
#include "parser/parser.hpp"
#include "utils/code_printer.hpp"

int main(int argc, char *argv[]) {
  cli::CliExecutor executor("boyo", "Boyo compiler");

  // Set usage string
  executor.set_usage("<input.boyo> [-o <output>] [--print-code]");

  // Add output flag
  executor.add_flag("-o,--output", cli::FlagType::MultiArg,
                    "Output file path (required unless --print-code is used)", false);

  // Add print-code flag
  executor.add_flag("--print-code", cli::FlagType::Boolean,
                    "Print generated C++ code without compiling", false);

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

    // Get output file (required unless --print-code is used)
    auto output_args = result.get_args("--output");
    if (!print_code && output_args.empty()) {
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
      if (print_code) {
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
