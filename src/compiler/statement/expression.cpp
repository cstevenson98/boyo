#include "statement/expression.hpp"

namespace boyo {

std::unique_ptr<Expression> CreateExpression(const std::string& token) {
  if (token == "print") {
    return std::make_unique<FunctionHeadExpression>(token);
  } else {
    return std::make_unique<LiteralExpression>(token);
  }
}

}  // namespace boyo
