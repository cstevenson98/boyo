#pragma once

#include <memory>
#include <string>
#include <vector>

#include "statement.hpp"

namespace boyo {

class Compiler {
 public:
  // Constructor
  Compiler();

  // Destructor
  ~Compiler();

  // Copy constructor
  Compiler(const Compiler& other);

  // Copy assignment operator
  Compiler& operator=(const Compiler& other);

  // Move constructor
  Compiler(Compiler&& other) noexcept;

  // Move assignment operator
  Compiler& operator=(Compiler&& other) noexcept;

  // Substitute the generated code into the main function
  static std::string SubstituteGeneratedCode(const std::string& main_function,
                                             const std::string& generated_code);

  // Generate the C++ code for the given statements
  static std::string GenerateProgramCode(
      const std::vector<std::unique_ptr<Statement>>& statements);

  // Compile the given lines into C++ code
  void compile(const std::vector<std::string>& lines,
               const std::string& output_file);

 private:
  int* data_;
};

}  // namespace boyo
