#include "statement/expression.hpp"

#include <sstream>
#include <stdexcept>

#include "lexer/lexer.hpp"

namespace boyo {

HexLiteralExpression::HexLiteralExpression(const std::string &hex_string)
    : hex_string_(hex_string) {
  // Validate hex string format: must start with "0x" and have at least one
  // digit
  if (hex_string.size() < 3 || !hex_string.starts_with("0x")) {
    throw std::runtime_error("Invalid hex literal: " + hex_string);
  }

  // Check that we have at least one hex digit after "0x"
  if (hex_string.substr(2).empty()) {
    throw std::runtime_error("Invalid hex literal: " + hex_string);
  }
}

std::string OperatorExpression::ToString() const {
  // Return Polish notation: "operator left right"
  std::ostringstream oss;
  oss << operator_ << " " << left_->ToString() << " " << right_->ToString();
  return oss.str();
}

std::unique_ptr<Expression> CreateExpression(const Lexer::Token &token) {
  using TokenType = Lexer::TokenType;

  switch (token.type_) {
  case TokenType::HEX_LITERAL:
    return std::make_unique<HexLiteralExpression>(token.value_);

  case TokenType::IDENTIFIER:
    return std::make_unique<IdentifierExpression>(token.value_);

  case TokenType::PARAM_IDENTIFIER:
    return std::make_unique<ParameterExpression>(token.value_);

  case TokenType::KEYWORD_LET:
  case TokenType::KEYWORD_DEF:
  case TokenType::KEYWORD_MAIN:
  case TokenType::KEYWORD_PRINT:
    return std::make_unique<KeywordExpression>(token.value_);

  case TokenType::OPERATOR_PLUS:
  case TokenType::OPERATOR_MINUS:
  case TokenType::OPERATOR_MULTIPLY:
    throw std::runtime_error(
        "Operator tokens must be parsed with ParsePolishExpression: " +
        token.value_);

  case TokenType::EQUALS:
  case TokenType::ARROW:
    throw std::runtime_error("Unexpected symbol token in expression: " +
                             token.value_);

  case TokenType::COMMENT:
  case TokenType::END_OF_FILE:
    throw std::runtime_error("Unexpected token in expression: " + token.value_);
  }

  throw std::runtime_error("Unknown token type");
}

std::unique_ptr<Expression>
ParsePolishExpression(const Lexer::TokenList &tokens, size_t &index) {
  if (index >= tokens.size()) {
    throw std::runtime_error("Unexpected end of tokens in expression");
  }

  using TokenType = Lexer::TokenType;
  const auto &token = tokens[index];

  // Check if current token is an operator
  if (token.type_ == TokenType::OPERATOR_PLUS ||
      token.type_ == TokenType::OPERATOR_MINUS ||
      token.type_ == TokenType::OPERATOR_MULTIPLY) {
    // Operator: recursively parse left and right operands
    std::string op = token.value_;
    index++; // Consume operator token

    // Recursively parse left operand
    auto left = ParsePolishExpression(tokens, index);

    // Recursively parse right operand
    auto right = ParsePolishExpression(tokens, index);

    return std::make_unique<OperatorExpression>(op, std::move(left),
                                                std::move(right));
  }

  // Base case: leaf expression (hex, identifier, parameter)
  index++; // Consume token
  return CreateExpression(tokens[index - 1]);
}

} // namespace boyo
