#include "parser/parser.hpp"

#include <memory>
#include <sstream>

#include "statement/expression.hpp"
#include "statement/statement.hpp"

namespace boyo {

// Simple statement implementation for now
class CommentStatement : public Statement {
public:
  explicit CommentStatement(const std::string &line) : line_(line) {}

  std::string GenerateCode() const override {
    // For now, just return a comment with the original line
    return "// " + line_ + "\n";
  }

private:
  std::string line_;
};

// Helper function to split a string into tokens
// For "print" statements, treats everything after "print" as a single literal
std::vector<std::string> SplitIntoTokens(const std::string &line) {
  std::vector<std::string> tokens;

  if (line.starts_with("print ")) {
    // Special handling for print: first token is "print", rest is literal
    tokens.push_back("print");
    if (line.length() > 6) {            // "print " is 6 chars
      tokens.push_back(line.substr(6)); // Everything after "print "
    } else {
      tokens.push_back(""); // Empty literal if just "print"
    }
  } else {
    // For other lines, split by whitespace
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

StatementList Parser::Parse(const std::vector<std::string> &lines) const {
  StatementList statements;

  for (const auto &line : lines) {
    if (line.empty()) {
      continue;
    }
    if (line.starts_with("//")) {
      statements.push_back(std::make_unique<CommentStatement>(line));
    } else {
      // Parse the line into tokens and create expressions
      auto tokens = SplitIntoTokens(line);
      ExpressionList expressions;
      for (const auto &token : tokens) {
        expressions.push_back(CreateExpression(token));
      }

      // Construct statement from expressions
      // For now, if first expression is "print", create PrintStatement
      if (!expressions.empty() && expressions[0]->ToString() == "print") {
        statements.push_back(
            std::make_unique<PrintStatement>(std::move(expressions)));
      } else {
        throw std::runtime_error("Invalid line: " + line);
      }
    }
  }
  return statements;
}

} // namespace boyo
