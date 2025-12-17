#include "compiler.hpp"

#include <cstdio>
#include <fstream>
#include <string>

#include "parser.hpp"

namespace boyo {

// C++ code snippets which will be inserted into the output.cpp file to be
// compiled into final executable

const std::string kBoyoProgramStartString = "{boyo_program_start}";
const std::string kBoyoProgramEndString = "{boyo_program_end}";

const std::string kMainFunctionSnippet =
    R"(
    #include <iostream>
    int main() {
        {boyo_program_start}
        {boyo_program_end}
        return 0;
    }
    )";

const std::string gpp_path = "/usr/bin/g++";

Compiler::Compiler() : data_(new int(42)) {}

Compiler::~Compiler() { delete data_; }

Compiler::Compiler(const Compiler& other) : data_(new int(*other.data_)) {}

Compiler& Compiler::operator=(const Compiler& other) {
  if (this != &other) {
    *data_ = *other.data_;
  }
  return *this;
}

Compiler::Compiler(Compiler&& other) noexcept : data_(other.data_) {
  other.data_ = nullptr;
}

Compiler& Compiler::operator=(Compiler&& other) noexcept {
  if (this != &other) {
    delete data_;
    data_ = other.data_;
    other.data_ = nullptr;
  }
  return *this;
}

// Substitute generated code into the main function
std::string Compiler::SubstituteGeneratedCode(
    const std::string& main_function, const std::string& generated_code) {
  std::string result = main_function;

  // Replace {boyo_program_start} with the generated code
  size_t start_pos = result.find(kBoyoProgramStartString);
  if (start_pos != std::string::npos) {
    result.replace(start_pos, kBoyoProgramStartString.length(), generated_code);
  }

  // Remove {boyo_program_end}
  size_t end_pos = result.find(kBoyoProgramEndString);
  if (end_pos != std::string::npos) {
    result.replace(end_pos, kBoyoProgramEndString.length(), "");
  }

  return result;
}

std::string Compiler::GenerateProgramCode(
    const std::vector<std::unique_ptr<Statement>>& statements) {
  std::string program_code;
  for (const auto& statement : statements) {
    program_code += statement->GenerateCode();
  }
  return program_code;
}

/**
  Compile the given lines of code into a binary executable
  @param lines The lines of code to compile
  @param output_file The path to the output file
  @return The path to the compiled executable
  @throws std::runtime_error if the program fails to compile
*/
void Compiler::compile(const std::vector<std::string>& lines,
                       const std::string& output_file) {
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
  std::string command =
      gpp_path + " -std=c++17 -o " + output_file + " " + temp_cpp_file;
  int result = system(command.c_str());

  // Clean up the temporary C++ file
  std::remove(temp_cpp_file.c_str());

  if (result != 0) {
    std::fprintf(stderr, "Error: Failed to compile program: %s\n",
                 output_file.c_str());
    throw std::runtime_error("Failed to compile program: " + output_file);
  }
}

}  // namespace boyo
