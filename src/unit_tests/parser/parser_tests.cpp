#include <gtest/gtest.h>

#include "parser/parser.hpp"

namespace boyo {
namespace {

// Test for Parser

TEST(ParserTest, Parse_ParsesComments) {
  std::vector<std::string> lines = {"// This is a comment",
                                    "print Hello World"};
  Parser parser;
  auto statements = parser.Parse(lines);
  EXPECT_EQ(statements.size(), 2);
  EXPECT_EQ(statements[0]->GenerateCode(), "// // This is a comment\n");
  EXPECT_EQ(statements[1]->GenerateCode(),
            "std::cout << \"Hello World\" << std::endl;\n");
}

}  // namespace
}  // namespace boyo

