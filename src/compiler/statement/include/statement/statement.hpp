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
  const ExpressionList &GetExpressions() const { return expressions_; }

protected:
  ExpressionList expressions_;
};

class PrintStatement : public Statement {
public:
  explicit PrintStatement(ExpressionList expressions);
  std::string GenerateCode() const override;
};

/**
 * Represents: let A = 0x10
 * Variable declaration with initialization
 */
class LetStatement : public Statement {
public:
  LetStatement(std::string var_name, std::unique_ptr<Expression> value_expr);
  std::string GenerateCode() const override;

  const std::string &GetVarName() const { return var_name_; }
  const Expression &GetValueExpr() const { return *value_expr_; }

private:
  std::string var_name_;
  std::unique_ptr<Expression> value_expr_;
};

/**
 * Represents: def double _a => * 0x10 _a
 * Function definition with parameters and body expression
 */
class DefStatement : public Statement {
public:
  DefStatement(std::string func_name, std::vector<std::string> params,
               std::unique_ptr<Expression> body_expr);
  std::string GenerateCode() const override;

  const std::string &GetFuncName() const { return func_name_; }
  const std::vector<std::string> &GetParams() const { return params_; }
  const Expression &GetBodyExpr() const { return *body_expr_; }

private:
  std::string func_name_;
  std::vector<std::string> params_;
  std::unique_ptr<Expression> body_expr_;
};

/**
 * Represents: main double A
 * Program entry point - calls a function and prints result
 */
class MainStatement : public Statement {
public:
  MainStatement(std::string func_name, std::vector<std::string> args);
  std::string GenerateCode() const override;

  const std::string &GetFuncName() const { return func_name_; }
  const std::vector<std::string> &GetArgs() const { return args_; }

private:
  std::string func_name_;
  std::vector<std::string> args_;
};

using StatementList = std::vector<std::unique_ptr<Statement>>;

} // namespace boyo
