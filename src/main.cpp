#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "cli.hpp"
#include "compiler/compiler.hpp"

int main(int argc, char* argv[]) {
  cli::CliExecutor executor("boyo", "Boyo compiler");

  // Set usage string
  executor.set_usage("<input.boyo> -o <output>");

  // Add output flag
  executor.add_flag

      ("-o,--output", cli::FlagType::MultiArg, "Output file path (required)",
       true);

  // Set handler for command-less mode
  executor.set_handler([](const cli::ParseResult& result) {
    // Get input file (first positional argument)
    if (result.positional_args.empty()) {
      std::fprintf(stderr, "Error: No input file specified\n");
      return 1;
    }

    const std::string& input_file = result.positional_args[0];

    // Get output file
    auto output_args = result.get_args("--output");
    if (output_args.empty()) {
      std::fprintf(stderr,
                   "Error: Output file not specified (use -o or --output)\n");
      return 1;
    }

    const std::string& output_file = output_args[0];

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

    // Compile the program
    try {
      boyo::Compiler compiler;
      compiler.compile(lines, output_file);
      return 0;
    } catch (const std::exception& e) {
      std::fprintf(stderr, "Error: %s\n", e.what());
      return 1;
    }
  });

  return executor.run(argc, argv);
}
