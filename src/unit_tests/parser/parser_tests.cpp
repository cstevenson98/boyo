#include <gtest/gtest.h>

#include "parser/parser.hpp"

namespace boyo {
namespace {

/**
 * Comment Parsing Tests
 */

TEST(ParserTest, ParseComments) {
  std::vector<std::string> lines = {"// This is a comment"};
  Parser parser;
  auto statements = parser.Parse(lines);
  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(), "//  This is a comment\n");
}

TEST(ParserTest, ParseEmptyLines) {
  std::vector<std::string> lines = {"", "// comment", ""};
  Parser parser;
  auto statements = parser.Parse(lines);
  EXPECT_EQ(statements.size(), 1);
}

/**
 * Let Statement Parsing Tests
 */

TEST(ParserTest, ParseLetStatement_Simple) {
  std::vector<std::string> lines = {"let A 0x10"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> A = {0x10};\n");
}

TEST(ParserTest, ParseLetStatement_MultiByteHex) {
  std::vector<std::string> lines = {"let result 0xDEADBEEF"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> result = {0xDEADBEEF};\n");
}

TEST(ParserTest, ParseLetStatement_InvalidMissingValue) {
  std::vector<std::string> lines = {"let A"};
  Parser parser;

  EXPECT_THROW(parser.Parse(lines), std::runtime_error);
}

TEST(ParserTest, ParseLetStatement_InvalidMissingIdentifier) {
  std::vector<std::string> lines = {"let 0x10"};
  Parser parser;

  EXPECT_THROW(parser.Parse(lines), std::runtime_error);
}

/**
 * Def Statement Parsing Tests
 */

TEST(ParserTest, ParseDefStatement_Simple) {
  std::vector<std::string> lines = {"def identity _a => _a"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> identity(const std::vector<uint8_t>& _a) {\n"
            "  return _a;\n"
            "}\n");
}

TEST(ParserTest, ParseDefStatement_WithOperator) {
  std::vector<std::string> lines = {"def double _a => * 0x10 _a"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> double(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors({0x10}, _a);\n"
            "}\n");
}

TEST(ParserTest, ParseDefStatement_NestedOperators) {
  std::vector<std::string> lines = {"def calc _a => * + 0x01 0x02 _a"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> calc(const std::vector<uint8_t>& _a) {\n"
            "  return multiply_vectors(add_vectors({0x01}, {0x02}), _a);\n"
            "}\n");
}

TEST(ParserTest, ParseDefStatement_MultipleParameters) {
  std::vector<std::string> lines = {"def add _a _b => + _a _b"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> add(const std::vector<uint8_t>& _a, const "
            "std::vector<uint8_t>& _b) {\n"
            "  return add_vectors(_a, _b);\n"
            "}\n");
}

TEST(ParserTest, ParseDefStatement_NoParameters) {
  std::vector<std::string> lines = {"def get_value => 0x42"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "std::vector<uint8_t> get_value() {\n"
            "  return {0x42};\n"
            "}\n");
}

TEST(ParserTest, ParseDefStatement_InvalidMissingArrow) {
  std::vector<std::string> lines = {"def double _a 0x10"};
  Parser parser;

  EXPECT_THROW(parser.Parse(lines), std::runtime_error);
}

/**
 * Main Statement Parsing Tests
 */

TEST(ParserTest, ParseMainStatement_SingleArg) {
  std::vector<std::string> lines = {"main double A"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "auto result = double(A);\n"
            "print_vector(std::cout, result);\n");
}

TEST(ParserTest, ParseMainStatement_MultipleArgs) {
  std::vector<std::string> lines = {"main compute A B C"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "auto result = compute(A, B, C);\n"
            "print_vector(std::cout, result);\n");
}

TEST(ParserTest, ParseMainStatement_NoArgs) {
  std::vector<std::string> lines = {"main get_value"};
  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 1);
  EXPECT_EQ(statements[0]->GenerateCode(),
            "auto result = get_value();\n"
            "print_vector(std::cout, result);\n");
}

TEST(ParserTest, ParseMainStatement_InvalidMissingFunction) {
  std::vector<std::string> lines = {"main"};
  Parser parser;

  EXPECT_THROW(parser.Parse(lines), std::runtime_error);
}

/**
 * Integration Tests - Complete Programs
 */

TEST(ParserTest, ParseCompleteProgram_Simple) {
  std::vector<std::string> lines = {"// Simple program", "let A 0x10",
                                    "def double _a => * 0x10 _a",
                                    "main double A"};

  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 4);

  // Verify comment
  EXPECT_EQ(statements[0]->GenerateCode(), "//  Simple program\n");

  // Verify let
  EXPECT_EQ(statements[1]->GenerateCode(),
            "std::vector<uint8_t> A = {0x10};\n");

  // Verify def
  std::string def_code = statements[2]->GenerateCode();
  EXPECT_TRUE(def_code.find("std::vector<uint8_t> double") !=
              std::string::npos);
  EXPECT_TRUE(def_code.find("multiply_vectors") != std::string::npos);

  // Verify main
  std::string main_code = statements[3]->GenerateCode();
  EXPECT_TRUE(main_code.find("double(A)") != std::string::npos);
}

TEST(ParserTest, ParseCompleteProgram_WithComments) {
  std::vector<std::string> lines = {
      "// Variable declaration", "let A 0xFF",    "// Function definition",
      "def identity _x => _x",   "// Main entry", "main identity A"};

  Parser parser;
  auto statements = parser.Parse(lines);

  EXPECT_EQ(statements.size(), 6);
}

TEST(ParserTest, ParseUnknownStatement) {
  std::vector<std::string> lines = {"unknown statement"};
  Parser parser;

  EXPECT_THROW(parser.Parse(lines), std::runtime_error);
}

} // namespace
} // namespace boyo
