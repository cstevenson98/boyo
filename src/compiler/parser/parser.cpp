#include "parser/parser.hpp"

#include <memory>
#include <sstream>

#include "lexer/lexer.hpp"
#include "statement/expression.hpp"
#include "statement/statement.hpp"

namespace boyo {

// Comment statement for preserving comments in output
class CommentStatement : public Statement {
public:
  explicit CommentStatement(const std::string &text) : text_(text) {}

  std::string GenerateCode() const override { return "// " + text_ + "\n"; }

private:
  std::string text_;
};

// Helper function to parse a let statement: let A 0x10
std::unique_ptr<Statement> ParseLetStatement(const Lexer::TokenList &tokens,
                                             size_t &index) {
  // Expect: let <identifier> <expression>
  if (index + 2 >= tokens.size()) {
    throw std::runtime_error("let statement requires identifier and value");
  }

  index++; // Skip 'let' keyword

  // Next token should be an identifier
  if (tokens[index].type_ != Lexer::TokenType::IDENTIFIER) {
    throw std::runtime_error("let statement requires identifier after 'let'");
  }
  std::string var_name = tokens[index].value_;
  index++;

  // Parse the value expression
  auto value_expr = ParsePolishExpression(tokens, index);

  return std::make_unique<LetStatement>(var_name, std::move(value_expr));
}

// Helper function to parse a def statement: def double _a => * 0x10 _a
std::unique_ptr<Statement> ParseDefStatement(const Lexer::TokenList &tokens,
                                             size_t &index) {
  // Expect: def <identifier> <params...> => <expression>
  if (index + 3 >= tokens.size()) {
    throw std::runtime_error(
        "def statement requires name, parameters, and body");
  }

  index++; // Skip 'def' keyword

  // Next token should be the function name
  if (tokens[index].type_ != Lexer::TokenType::IDENTIFIER) {
    throw std::runtime_error(
        "def statement requires function name after 'def'");
  }
  std::string func_name = tokens[index].value_;
  index++;

  // Collect parameters (all PARAM_IDENTIFIER tokens before '=>')
  std::vector<std::string> params;
  while (index < tokens.size() &&
         tokens[index].type_ != Lexer::TokenType::ARROW) {
    if (tokens[index].type_ == Lexer::TokenType::PARAM_IDENTIFIER) {
      params.push_back(tokens[index].value_);
      index++;
    } else {
      throw std::runtime_error("Expected parameter or '=>' in def statement");
    }
  }

  // Expect '=>'
  if (index >= tokens.size() ||
      tokens[index].type_ != Lexer::TokenType::ARROW) {
    throw std::runtime_error("def statement requires '=>' before body");
  }
  index++; // Skip '=>'

  // Parse the body expression
  auto body_expr = ParsePolishExpression(tokens, index);

  return std::make_unique<DefStatement>(func_name, params,
                                        std::move(body_expr));
}

// Helper function to parse a main statement: main double A
std::unique_ptr<Statement> ParseMainStatement(const Lexer::TokenList &tokens,
                                              size_t &index) {
  // Expect: main <identifier> <args...>
  if (index + 1 >= tokens.size()) {
    throw std::runtime_error("main statement requires function name");
  }

  index++; // Skip 'main' keyword

  // Next token should be the function name
  if (tokens[index].type_ != Lexer::TokenType::IDENTIFIER) {
    throw std::runtime_error(
        "main statement requires function name after 'main'");
  }
  std::string func_name = tokens[index].value_;
  index++;

  // Collect all remaining identifiers as arguments
  std::vector<std::string> args;
  while (index < tokens.size() &&
         tokens[index].type_ != Lexer::TokenType::END_OF_FILE) {
    if (tokens[index].type_ == Lexer::TokenType::IDENTIFIER) {
      args.push_back(tokens[index].value_);
      index++;
    } else {
      throw std::runtime_error(
          "Expected identifier in main statement arguments");
    }
  }

  return std::make_unique<MainStatement>(func_name, args);
}

// Parse a single line into a statement
std::unique_ptr<Statement> ParseLine(const std::string &line) {
  Lexer lexer;
  auto tokens = lexer.Tokenize({line});

  if (tokens.empty()) {
    return nullptr;
  }

  // Handle comments
  if (tokens[0].type_ == Lexer::TokenType::COMMENT) {
    // Remove leading "//" from comment text
    std::string comment_text = tokens[0].value_;
    if (comment_text.starts_with("//")) {
      comment_text = comment_text.substr(2);
    }
    return std::make_unique<CommentStatement>(comment_text);
  }

  size_t index = 0;

  // Determine statement type based on first token
  if (tokens[0].type_ == Lexer::TokenType::KEYWORD_LET) {
    return ParseLetStatement(tokens, index);
  } else if (tokens[0].type_ == Lexer::TokenType::KEYWORD_DEF) {
    return ParseDefStatement(tokens, index);
  } else if (tokens[0].type_ == Lexer::TokenType::KEYWORD_MAIN) {
    return ParseMainStatement(tokens, index);
  } else if (tokens[0].type_ == Lexer::TokenType::KEYWORD_PRINT) {
    // Old print statement - could implement if needed
    throw std::runtime_error(
        "print statement not yet implemented in new parser");
  } else {
    throw std::runtime_error("Unknown statement type: " + tokens[0].value_);
  }
}

StatementList Parser::Parse(const std::vector<std::string> &lines) const {
  StatementList statements;

  for (const auto &line : lines) {
    if (line.empty()) {
      continue;
    }

    auto statement = ParseLine(line);
    if (statement) {
      statements.push_back(std::move(statement));
    }
  }

  return statements;
}

} // namespace boyo
