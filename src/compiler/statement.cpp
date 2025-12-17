#include "statement.hpp"

namespace boyo {

/**
 * Constructor for PrintStatement
 * @brief This type of statement has the form "print <byte_string>"
 * @param code_line The code line to print. This is the byte string to print.
 */
PrintStatement::PrintStatement(const std::vector<std::uint8_t>& byte_string)
    : byte_string_(byte_string) {}

// Generate the C++ for printing the byte string in ascii, by casting them to
// chars
std::string PrintStatement::GenerateCode() const {
  std::string code = "std::cout << \"";
  for (const auto& byte : byte_string_) {
    code += static_cast<char>(byte);
  }
  code += "\" << std::endl;\n";
  return code;
}

}  // namespace boyo
