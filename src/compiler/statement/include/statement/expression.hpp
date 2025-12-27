#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Forward declare Token from lexer
namespace boyo {
struct Token;
using TokenList = std::vector<Token>;
} // namespace boyo

#include "lexer/lexer.hpp"

namespace boyo {

class Expression {
public:
  virtual ~Expression() = default;

  // Get the string representation of this expression
  virtual std::string ToString() const = 0;
};

/**
 * Represents hex literals like 0x10, 0x1234
 * Stores the hex string for code generation
 */
class HexLiteralExpression : public Expression {
public:
  explicit HexLiteralExpression(const std::string &hex_string);

  std::string ToString() const override { return hex_string_; }

  const std::string &GetHexString() const { return hex_string_; }

private:
  std::string hex_string_; // Original string (e.g., "0x10")
};

/**
 * Represents user-defined identifiers (variables, function names)
 * Examples: A, double, myVar
 */
class IdentifierExpression : public Expression {
public:
  explicit IdentifierExpression(const std::string &name) : name_(name) {}

  std::string ToString() const override { return name_; }

  const std::string &GetName() const { return name_; }

private:
  std::string name_;
};

/**
 * Represents function parameters starting with underscore
 * Examples: _a, _b, _param
 */
class ParameterExpression : public Expression {
public:
  explicit ParameterExpression(const std::string &param_name)
      : param_name_(param_name) {}

  std::string ToString() const override { return param_name_; }

  const std::string &GetParamName() const { return param_name_; }

private:
  std::string param_name_;
};

/**
 * Represents Polish notation operator with two operands
 * Example: * 0x10 _a
 */
class OperatorExpression : public Expression {
public:
  OperatorExpression(const std::string &op, std::unique_ptr<Expression> left,
                     std::unique_ptr<Expression> right)
      : operator_(op), left_(std::move(left)), right_(std::move(right)) {}

  std::string ToString() const override;

  const std::string &GetOperator() const { return operator_; }
  const Expression &GetLeft() const { return *left_; }
  const Expression &GetRight() const { return *right_; }

private:
  std::string operator_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

/**
 * Represents keywords (let, print, def, main)
 * Used during statement construction
 */
class KeywordExpression : public Expression {
public:
  explicit KeywordExpression(const std::string &keyword) : keyword_(keyword) {}

  std::string ToString() const override { return keyword_; }

  const std::string &GetKeyword() const { return keyword_; }

private:
  std::string keyword_;
};

using ExpressionList = std::vector<std::unique_ptr<Expression>>;

/**
 * Create an Expression from a single Token
 * Handles leaf expressions: hex literals, identifiers, parameters, keywords
 * Throws for operator tokens (must use ParsePolishExpression)
 */
std::unique_ptr<Expression> CreateExpression(const Lexer::Token &token);

/**
 * Parse Polish notation expression from tokens
 * Recursively builds OperatorExpression tree for nested operators
 * @param tokens The token list to parse
 * @param index Current position (will be advanced as tokens are consumed)
 * @return Expression AST (OperatorExpression for operators, leaf expression
 * otherwise)
 */
std::unique_ptr<Expression>
ParsePolishExpression(const Lexer::TokenList &tokens, size_t &index);

} // namespace boyo
