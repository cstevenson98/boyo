#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "compiler/compiler.hpp"
#include "parser/parser.hpp"

namespace boyo {
namespace {

class CompilerTest : public ::testing::Test {
 protected:
  void SetUp() override { compiler = std::make_unique<Compiler>(); }

  std::unique_ptr<Compiler> compiler;
};

TEST_F(CompilerTest, TestGenerateProgramCode_GeneratesProgramCode) {
  std::vector<std::string> lines = {"print Hello World"};
  auto statements = Parser().Parse(lines);
  auto program_code = Compiler::GenerateProgramCode(statements);
  EXPECT_EQ(program_code, "std::cout << \"Hello World\" << std::endl;\n");
}

TEST_F(CompilerTest, TestSubstituteGeneratedCode_SubstitutesGeneratedCode) {
  std::string main_function = R"(
    #include <iostream>
    int main() {
      {boyo_program_start}
      {boyo_program_end}
    }
  )";

  auto generated_code = "std::cout << \"Hello World\" << std::endl;\n";
  auto substituted_code =
      Compiler::SubstituteGeneratedCode(main_function, generated_code);
  // After substitution: {boyo_program_start} is replaced with generated_code,
  // and {boyo_program_end} is removed, leaving the indentation that was before
  // it
  EXPECT_EQ(substituted_code,
            "\n    #include <iostream>\n    int main() {\n      std::cout << "
            "\"Hello World\" << std::endl;\n\n      \n    }\n  ");
}

TEST_F(CompilerTest, Compile_CompilesProgram) {
  std::vector<std::string> lines = {"print Hello World"};
  compiler->compile(lines, "test_program");
  SUCCEED();
}

}  // namespace
}  // namespace boyo

