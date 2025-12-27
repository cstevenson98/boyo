#include "statement/statement.hpp"

#include <sstream>

#include "statement/expression.hpp"

namespace boyo {

// Forward declaration
std::string GenerateExpressionCode(const Expression *expr);

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
  for (const auto &byte : literal_value) {
    code += static_cast<char>(byte);
  }
  code += "\" << std::endl;\n";
  return code;
}

LetStatement::LetStatement(std::string var_name,
                           std::unique_ptr<Expression> value_expr)
    : var_name_(std::move(var_name)), value_expr_(std::move(value_expr)) {}

std::string LetStatement::GenerateCode() const {
  // Generate: std::vector<uint8_t> A = {0x10};
  std::ostringstream oss;
  oss << "std::vector<uint8_t> " << var_name_ << " = {";

  // Get the hex value from the expression
  if (auto *hex_expr =
          dynamic_cast<const HexLiteralExpression *>(value_expr_.get())) {
    oss << hex_expr->GetHexString();
  } else {
    // For now, assume it's a simple identifier or will be handled later
    oss << value_expr_->ToString();
  }

  oss << "};\n";
  return oss.str();
}

DefStatement::DefStatement(std::string func_name,
                           std::vector<std::string> params,
                           std::unique_ptr<Expression> body_expr)
    : func_name_(std::move(func_name)), params_(std::move(params)),
      body_expr_(std::move(body_expr)) {}

std::string DefStatement::GenerateCode() const {
  // Generate: std::vector<uint8_t> double(const std::vector<uint8_t>& _a) { ...
  // }
  std::ostringstream oss;
  oss << "std::vector<uint8_t> " << func_name_ << "(";

  // Generate parameters
  for (size_t i = 0; i < params_.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << "const std::vector<uint8_t>& " << params_[i];
  }

  oss << ") {\n";
  oss << "  return " << GenerateExpressionCode(body_expr_.get()) << ";\n";
  oss << "}\n";

  return oss.str();
}

MainStatement::MainStatement(std::string func_name,
                             std::vector<std::string> args)
    : func_name_(std::move(func_name)), args_(std::move(args)) {}

std::string MainStatement::GenerateCode() const {
  // Generate: auto result = double(A); print_vector(std::cout, result);
  std::ostringstream oss;

  oss << "auto result = " << func_name_ << "(";

  // Generate arguments
  for (size_t i = 0; i < args_.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << args_[i];
  }

  oss << ");\n";
  oss << "print_vector(std::cout, result);\n";

  return oss.str();
}

// Helper function to generate code for expressions (especially operators)
std::string GenerateExpressionCode(const Expression *expr) {
  if (auto *op_expr = dynamic_cast<const OperatorExpression *>(expr)) {
    // Generate operator function call: multiply_vectors(left, right)
    std::string op_func;
    if (op_expr->GetOperator() == "*") {
      op_func = "multiply_vectors";
    } else if (op_expr->GetOperator() == "+") {
      op_func = "add_vectors";
    } else if (op_expr->GetOperator() == "-") {
      op_func = "subtract_vectors";
    } else {
      throw std::runtime_error("Unknown operator: " + op_expr->GetOperator());
    }

    return op_func + "(" + GenerateExpressionCode(&op_expr->GetLeft()) + ", " +
           GenerateExpressionCode(&op_expr->GetRight()) + ")";
  }

  if (auto *hex_expr = dynamic_cast<const HexLiteralExpression *>(expr)) {
    // Generate: {0x10}
    return "{" + hex_expr->GetHexString() + "}";
  }

  if (auto *param_expr = dynamic_cast<const ParameterExpression *>(expr)) {
    // Generate: _a (parameter reference)
    return param_expr->GetParamName();
  }

  if (auto *id_expr = dynamic_cast<const IdentifierExpression *>(expr)) {
    // Generate: identifier name
    return id_expr->GetName();
  }

  throw std::runtime_error("Unknown expression type in code generation");
}

} // namespace boyo
