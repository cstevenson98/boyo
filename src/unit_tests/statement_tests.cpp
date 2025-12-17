#include <gtest/gtest.h>

#include <memory>

#include "statement.hpp"

namespace boyo {
namespace {

// Test for PrintStatement

TEST(PrintStatementTest, GenerateCode_PrintsByteString) {
  std::vector<std::uint8_t> byte_string = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                           0x57, 0x6f, 0x72, 0x6c, 0x64};
  PrintStatement print_statement(byte_string);
  std::string code = print_statement.GenerateCode();
  EXPECT_EQ(code, "std::cout << \"Hello World\" << std::endl;");
}

}  // namespace
}  // namespace boyo