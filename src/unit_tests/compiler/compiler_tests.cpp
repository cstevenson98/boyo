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

TEST_F(CompilerTest, TestGenerateProgramCode_SimpleLetStatement) {
  std::vector<std::string> lines = {"let A 0x10"};
  auto statements = Parser().Parse(lines);
  auto program_code = Compiler::GenerateProgramCode(statements);
  EXPECT_TRUE(program_code.find("std::vector<uint8_t> A = {0x10}") !=
              std::string::npos);
}

TEST_F(CompilerTest, TestGenerateProgramCode_CompleteProgram) {
  std::vector<std::string> lines = {"let A 0x10", "def double _a => * 0x10 _a",
                                    "main double A"};
  auto statements = Parser().Parse(lines);
  auto program_code = Compiler::GenerateProgramCode(statements);

  // Verify it contains all parts
  EXPECT_TRUE(program_code.find("std::vector<uint8_t> A = {0x10}") !=
              std::string::npos);
  EXPECT_TRUE(program_code.find("std::vector<uint8_t> double") !=
              std::string::npos);
  EXPECT_TRUE(program_code.find("multiply_vectors") != std::string::npos);
  EXPECT_TRUE(program_code.find("auto result = double(A)") !=
              std::string::npos);
  // Verify split point exists
  EXPECT_TRUE(program_code.find("{boyo_split_point}") != std::string::npos);
}

TEST_F(CompilerTest, TestSubstituteGeneratedCode_SubstitutesGeneratedCode) {
  std::string main_function = R"(
    #include <iostream>
    #include <vector>
    int main() {
      {boyo_program_start}
      {boyo_program_end}
    }
  )";

  auto generated_code = "std::vector<uint8_t> A = {0x10};\n";
  auto substituted_code =
      Compiler::SubstituteGeneratedCode(main_function, generated_code);
  // After substitution: {boyo_program_start} is replaced with generated_code,
  // and {boyo_program_end} is removed
  EXPECT_TRUE(substituted_code.find("std::vector<uint8_t> A = {0x10};") !=
              std::string::npos);
  EXPECT_TRUE(substituted_code.find("{boyo_program_start}") ==
              std::string::npos);
  EXPECT_TRUE(substituted_code.find("{boyo_program_end}") == std::string::npos);
}

TEST_F(CompilerTest, Compile_CompilesProgram) {
  std::vector<std::string> lines = {"let A 0x10", "def identity _x => _x",
                                    "main identity A"};
  compiler->compile(lines, "test_program");
  SUCCEED();
}

} // namespace
} // namespace boyo
