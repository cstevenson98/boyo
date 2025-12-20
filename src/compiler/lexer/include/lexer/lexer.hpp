#pragma once

#include <string>
#include <vector>

namespace boyo {

// Forward declaration for test friend class
class LexerTest;

/**
 * @brief The Lexer class is responsible for tokenizing the input lines into a
 * list of tokens
 */
class Lexer {
public:
  enum class TokenType {
    KEYWORD_LET,       // let
    KEYWORD_DEF,       // def
    KEYWORD_MAIN,      // main
    IDENTIFIER,        // A-Z
    PARAM_IDENTIFIER,  // _a, _b, _c, etc.
    OPERATOR_PLUS,     // +
    OPERATOR_MINUS,    // -
    OPERATOR_MULTIPLY, // *
    HEX_LITERAL,       // 0x1234
    UNKNOWN,           // Unknown token
    COMMENT,           // // comment
    END_OF_FILE,       // End of file
  };

  struct Token {
    TokenType type_;
    std::string value_;
    size_t line_;
    size_t column_;
  };

  using TokenList = std::vector<Token>;

  /**
   * Tokenize the input lines into a list of tokens
   * @param lines The input lines to tokenize
   * @return A list of tokens
   */
  TokenList Tokenize(const std::vector<std::string> &lines) const;

private:
  /**
   * Classify the token string into a token type
   * @param token_string The token string to classify
   * @return The token type
   */
  TokenType ClassifyToken(const std::string &token_string) const;

  /**
   * Tokenize a single line into a list of tokens
   * @param line The line to tokenize
   * @param line_number The line number
   * @return A list of tokens
   */
  TokenList TokenizeLine(const std::string &line, size_t line_number) const;

  // Allow test class to access private methods
  friend class LexerTest;
};

} // namespace boyo
