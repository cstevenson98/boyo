#include "statement/statement.hpp"

#include "statement/expression.hpp"

namespace boyo {

/**
 * Constructor for PrintStatement
 * @brief This type of statement has the form "print <literal>"
 * @param expressions The expressions in this statement (should be [print,
 * literal])
 */
PrintStatement::PrintStatement(ExpressionList expressions) {
  expressions_ = std::move(expressions);
}

// Generate the C++ for printing the literal expression
std::string PrintStatement::GenerateCode() const {
  if (expressions_.size() < 2) {
    throw std::runtime_error("PrintStatement requires at least 2 expressions");
  }

  // First expression should be "print", second should be the literal
  std::string literal_value = expressions_[1]->ToString();

  std::string code = "std::cout << \"";
  for (const auto& byte : literal_value) {
    code += static_cast<char>(byte);
  }
  code += "\" << std::endl;\n";
  return code;
}

}  // namespace boyo
