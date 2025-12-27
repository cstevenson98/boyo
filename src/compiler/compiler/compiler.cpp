#include "compiler/compiler.hpp"

#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "parser/parser.hpp"
#include "statement/statement.hpp"

namespace boyo {

// C++ code snippets which will be inserted into the output.cpp file to be
// compiled into final executable

const std::string kBoyoProgramStartString = "{boyo_program_start}";
const std::string kBoyoProgramEndString = "{boyo_program_end}";

const std::string kMainFunctionSnippet =
    R"(
    #include <iostream>
    #include <vector>
    #include <cstdint>
    
    // Helper function to print vectors
    void print_vector(std::ostream& os, const std::vector<uint8_t>& vec) {
        for (const auto& byte : vec) {
            os << std::hex << static_cast<int>(byte) << " ";
        }
        os << std::dec << std::endl;
    }
    
    // Helper functions for vector operations
    std::vector<uint8_t> add_vectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
        std::vector<uint8_t> result;
        size_t max_size = std::max(a.size(), b.size());
        result.reserve(max_size);
        for (size_t i = 0; i < max_size; ++i) {
            uint8_t val_a = (i < a.size()) ? a[i] : 0;
            uint8_t val_b = (i < b.size()) ? b[i] : 0;
            result.push_back(val_a + val_b);
        }
        return result;
    }
    
    std::vector<uint8_t> subtract_vectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
        std::vector<uint8_t> result;
        size_t max_size = std::max(a.size(), b.size());
        result.reserve(max_size);
        for (size_t i = 0; i < max_size; ++i) {
            uint8_t val_a = (i < a.size()) ? a[i] : 0;
            uint8_t val_b = (i < b.size()) ? b[i] : 0;
            result.push_back(val_a - val_b);
        }
        return result;
    }
    
    std::vector<uint8_t> multiply_vectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
        std::vector<uint8_t> result;
        size_t max_size = std::max(a.size(), b.size());
        result.reserve(max_size);
        for (size_t i = 0; i < max_size; ++i) {
            uint8_t val_a = (i < a.size()) ? a[i] : 0;
            uint8_t val_b = (i < b.size()) ? b[i] : 0;
            result.push_back(val_a * val_b);
        }
        return result;
    }
    
    {boyo_program_start}
    
    int main() {
        {boyo_program_end}
        return 0;
    }
    )";

const std::string gpp_path = "/usr/bin/g++";

Compiler::Compiler() : data_(new int(42)) {}

Compiler::~Compiler() { delete data_; }

Compiler::Compiler(const Compiler &other) : data_(new int(*other.data_)) {}

Compiler &Compiler::operator=(const Compiler &other) {
  if (this != &other) {
    *data_ = *other.data_;
  }
  return *this;
}

Compiler::Compiler(Compiler &&other) noexcept : data_(other.data_) {
  other.data_ = nullptr;
}

Compiler &Compiler::operator=(Compiler &&other) noexcept {
  if (this != &other) {
    delete data_;
    data_ = other.data_;
    other.data_ = nullptr;
  }
  return *this;
}

// Substitute generated code into the main function
std::string
Compiler::SubstituteGeneratedCode(const std::string &main_function,
                                  const std::string &generated_code) {
  std::string result = main_function;

  // Split generated_code into global and main parts
  std::string global_code;
  std::string main_code;

  size_t split_pos = generated_code.find("{boyo_split_point}");
  if (split_pos != std::string::npos) {
    global_code = generated_code.substr(0, split_pos);
    main_code = generated_code.substr(
        split_pos + 18); // 18 is length of "{boyo_split_point}"
  } else {
    // No split point, put everything as global
    global_code = generated_code;
  }

  // Replace {boyo_program_start} with global code (variables and functions)
  size_t start_pos = result.find(kBoyoProgramStartString);
  if (start_pos != std::string::npos) {
    result.replace(start_pos, kBoyoProgramStartString.length(), global_code);
  }

  // Replace {boyo_program_end} with main code
  size_t end_pos = result.find(kBoyoProgramEndString);
  if (end_pos != std::string::npos) {
    result.replace(end_pos, kBoyoProgramEndString.length(), main_code);
  }

  return result;
}

std::string Compiler::GenerateProgramCode(const StatementList &statements) {
  std::string global_code; // Variables and functions
  std::string main_code;   // Main execution code

  for (const auto &statement : statements) {
    // Check if this is a MainStatement by trying to dynamic_cast
    if (dynamic_cast<const MainStatement *>(statement.get())) {
      main_code += statement->GenerateCode();
    } else {
      global_code += statement->GenerateCode();
    }
  }

  return global_code + "{boyo_split_point}" + main_code;
}

std::string Compiler::GetMainFunctionSnippet() { return kMainFunctionSnippet; }

/**
  Compile the given lines of code into a binary executable
  @param lines The lines of code to compile
  @param output_file The path to the output file
  @return The path to the compiled executable
  @throws std::runtime_error if the program fails to compile
*/
void Compiler::compile(const std::vector<std::string> &lines,
                       const std::string &output_file) {
  Parser parser;
  auto statements = parser.Parse(lines);

  auto program_code = GenerateProgramCode(statements);

  auto main_function =
      SubstituteGeneratedCode(kMainFunctionSnippet, program_code);

  // Write the C++ code to a temporary file
  std::string temp_cpp_file = output_file + ".cpp";
  std::ofstream cpp_out(temp_cpp_file);
  if (!cpp_out) {
    std::fprintf(stderr, "Error: Failed to create temporary C++ file: %s\n",
                 temp_cpp_file.c_str());
    throw std::runtime_error("Failed to create temporary C++ file: " +
                             temp_cpp_file);
  }
  cpp_out << main_function;
  cpp_out.close();

  // Compile the C++ file to the output binary
  std::string command = gpp_path + " -std=c++17 -o " + output_file + " " +
                        temp_cpp_file + " 2>&1";

  // Capture compiler output
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    std::remove(temp_cpp_file.c_str());
    std::fprintf(stderr, "Error: Failed to execute compiler command\n");
    throw std::runtime_error("Failed to execute compiler command");
  }

  std::string compiler_output;
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    compiler_output += buffer;
  }

  int result = pclose(pipe);
  int exit_code = 0;
  if (WIFEXITED(result)) {
    exit_code = WEXITSTATUS(result);
  } else {
    exit_code = result;
  }

  // Clean up the temporary C++ file
  std::remove(temp_cpp_file.c_str());

  if (exit_code != 0) {
    // Log boyo error first
    std::fprintf(stderr, "Error: Failed to compile program: %s\n",
                 output_file.c_str());

    // Then log compiler output with proper formatting
    if (!compiler_output.empty()) {
      std::fprintf(stderr, "\nCompiler output:\n");
      std::istringstream iss(compiler_output);
      std::string line;
      while (std::getline(iss, line)) {
        // Remove trailing newline if present (getline already does this, but be
        // safe)
        if (!line.empty() && line.back() == '\n') {
          line.pop_back();
        }
        std::fprintf(stderr, "| %s\n", line.c_str());
      }
    }

    throw std::runtime_error("Failed to compile program: " + output_file);
  }
}

} // namespace boyo
