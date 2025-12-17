#include "parser.hpp"

#include <memory>

#include "statement.hpp"

namespace boyo {

// Simple statement implementation for now
class CommentStatement : public Statement {
 public:
  explicit CommentStatement(const std::string& line) : line_(line) {}

  std::string GenerateCode() const override {
    // For now, just return a comment with the original line
    return "// " + line_ + "\n";
  }

 private:
  std::string line_;
};

std::vector<std::unique_ptr<Statement>> Parser::Parse(
    const std::vector<std::string>& lines) const {
  std::vector<std::unique_ptr<Statement>> statements;

  for (const auto& line : lines) {
    if (line.empty()) {
      continue;
    }
    if (line.starts_with("//")) {
      statements.push_back(std::make_unique<CommentStatement>(line));
    } else if (line.starts_with("print")) {
      // Get the byte string from the line
      std::string byte_string = line.substr(5);
      // Convert the byte string to a vector of uint8_t
      std::vector<std::uint8_t> byte_vector;
      for (const auto& byte : byte_string) {
        byte_vector.push_back(static_cast<std::uint8_t>(byte));
      }
      statements.push_back(std::make_unique<PrintStatement>(byte_vector));
    } else {
      throw std::runtime_error("Invalid line: " + line);
    }
  }
  return statements;
}
}  // namespace boyo
