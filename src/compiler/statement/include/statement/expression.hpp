#pragma once

#include <memory>
#include <string>
#include <vector>

namespace boyo {

class Expression {
 public:
  virtual ~Expression() = default;

  // Get the string representation of this expression
  virtual std::string ToString() const = 0;
};

class FunctionHeadExpression : public Expression {
 public:
  explicit FunctionHeadExpression(const std::string& name) : name_(name) {}

  std::string ToString() const override { return name_; }

  const std::string& GetName() const { return name_; }

 private:
  std::string name_;
};

class LiteralExpression : public Expression {
 public:
  explicit LiteralExpression(const std::string& value) : value_(value) {}

  std::string ToString() const override { return value_; }

  const std::string& GetValue() const { return value_; }

 private:
  std::string value_;
};

using ExpressionList = std::vector<std::unique_ptr<Expression>>;

// Factory function to create an Expression from a string
std::unique_ptr<Expression> CreateExpression(const std::string& token);

}  // namespace boyo
