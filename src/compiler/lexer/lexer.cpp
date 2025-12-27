#include <sstream>

#include "lexer/lexer.hpp"

namespace boyo {

Lexer::TokenType Lexer::ClassifyToken(const std::string &token_string) const {
  if (token_string == "let") {
    return TokenType::KEYWORD_LET;
  }
  if (token_string == "def") {
    return TokenType::KEYWORD_DEF;
  }
  if (token_string == "main") {
    return TokenType::KEYWORD_MAIN;
  }
  if (token_string == "print") {
    return TokenType::KEYWORD_PRINT;
  }
  if (token_string == "=>") {
    return TokenType::ARROW;
  }
  if (token_string == "=") {
    return TokenType::EQUALS;
  }
  if (token_string == "+") {
    return TokenType::OPERATOR_PLUS;
  }
  if (token_string == "-") {
    return TokenType::OPERATOR_MINUS;
  }
  if (token_string == "*") {
    return TokenType::OPERATOR_MULTIPLY;
  }
  if (token_string.starts_with("0x")) {
    return TokenType::HEX_LITERAL;
  }
  if (token_string.starts_with("_")) {
    return TokenType::PARAM_IDENTIFIER;
  }
  if (token_string.starts_with("//")) {
    return TokenType::COMMENT;
  }
  if (token_string.empty()) {
    return TokenType::END_OF_FILE;
  }
  // Default: treat as user-defined identifier (variable or function name)
  return TokenType::IDENTIFIER;
}

Lexer::TokenList Lexer::TokenizeLine(const std::string &line,
                                     size_t line_number) const {
  TokenList tokens;

  // Find comment position
  const size_t comment_pos = line.find("//");

  // Extract the part before the comment (or whole line if no comment)
  std::string code_part =
      (comment_pos != std::string::npos) ? line.substr(0, comment_pos) : line;

  // Tokenize the code part
  std::istringstream iss(code_part);
  std::string token_string;
  size_t column_number = 0;
  while (iss >> token_string) {
    const auto token_type = ClassifyToken(token_string);
    tokens.push_back(
        Token{token_type, token_string, line_number, column_number});
    column_number += token_string.length() + 1;
  }

  // Add comment token if present
  if (comment_pos != std::string::npos) {
    std::string comment_text = line.substr(comment_pos);
    tokens.push_back(
        Token{TokenType::COMMENT, comment_text, line_number, comment_pos});
  }

  return tokens;
}

Lexer::TokenList Lexer::Tokenize(const std::vector<std::string> &lines) const {
  TokenList tokens;

  for (size_t line_number = 0; line_number < lines.size(); ++line_number) {
    const auto &line = lines[line_number];
    const auto &line_tokens = TokenizeLine(line, line_number);
    tokens.insert(tokens.end(), line_tokens.begin(), line_tokens.end());
  }

  return tokens;
}
} // namespace boyo
