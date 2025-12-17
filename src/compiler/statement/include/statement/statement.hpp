#pragma once

#include <memory>
#include <string>
#include <vector>

#include "statement/expression.hpp"

namespace boyo {

class Statement {
 public:
  virtual ~Statement() = default;

  // Generate the C++ code for this statement
  virtual std::string GenerateCode() const = 0;

  // Get the expressions in this statement
  const ExpressionList& GetExpressions() const { return expressions_; }

 protected:
  ExpressionList expressions_;
};

class PrintStatement : public Statement {
 public:
  explicit PrintStatement(ExpressionList expressions);
  std::string GenerateCode() const override;
};

}  // namespace boyo
