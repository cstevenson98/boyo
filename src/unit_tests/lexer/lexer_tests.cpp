#include <gtest/gtest.h>
#include <memory>

#include "lexer/lexer.hpp"

namespace boyo {

// Test fixture class - friend of Lexer to access private methods
class LexerTest : public ::testing::Test {
protected:
  void SetUp() override { lexer = std::make_unique<Lexer>(); }

  // Helper method to access private ClassifyToken method
  Lexer::TokenType ClassifyToken(const std::string &token_string) const {
    return lexer->ClassifyToken(token_string);
  }

  // Helper method to access private TokenizeLine method
  Lexer::TokenList TokenizeLine(const std::string &line,
                                size_t line_number) const {
    return lexer->TokenizeLine(line, line_number);
  }

  std::unique_ptr<Lexer> lexer;
};

namespace {

TEST_F(LexerTest, ClassifyToken_AllCases) {
  // Test keywords
  EXPECT_EQ(ClassifyToken("let"), Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(ClassifyToken("def"), Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(ClassifyToken("main"), Lexer::TokenType::KEYWORD_MAIN);

  // Test operators
  EXPECT_EQ(ClassifyToken("+"), Lexer::TokenType::OPERATOR_PLUS);
  EXPECT_EQ(ClassifyToken("-"), Lexer::TokenType::OPERATOR_MINUS);
  EXPECT_EQ(ClassifyToken("*"), Lexer::TokenType::OPERATOR_MULTIPLY);

  // Test hex literals (starts with "0x")
  EXPECT_EQ(ClassifyToken("0x"), Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(ClassifyToken("0x1234"), Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(ClassifyToken("0xABCD"), Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(ClassifyToken("0x0"), Lexer::TokenType::HEX_LITERAL);

  // Test param identifiers (starts with "_")
  EXPECT_EQ(ClassifyToken("_"), Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(ClassifyToken("_a"), Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(ClassifyToken("_b"), Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(ClassifyToken("_param"), Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(ClassifyToken("_123"), Lexer::TokenType::PARAM_IDENTIFIER);

  // Test comments (starts with "//")
  EXPECT_EQ(ClassifyToken("//"), Lexer::TokenType::COMMENT);
  EXPECT_EQ(ClassifyToken("// comment"), Lexer::TokenType::COMMENT);
  EXPECT_EQ(ClassifyToken("// This is a comment"), Lexer::TokenType::COMMENT);

  // Test end of file (empty string)
  EXPECT_EQ(ClassifyToken(""), Lexer::TokenType::END_OF_FILE);

  // Test unknown tokens
  EXPECT_EQ(ClassifyToken("abc"), Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(ClassifyToken("123"), Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(ClassifyToken("variable"), Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(ClassifyToken("="), Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(ClassifyToken("("), Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(ClassifyToken(")"), Lexer::TokenType::UNKNOWN);

  // Test edge cases that should be UNKNOWN
  EXPECT_EQ(ClassifyToken("letx"),
            Lexer::TokenType::UNKNOWN); // "let" prefix but not exact match
  EXPECT_EQ(ClassifyToken("0x"),
            Lexer::TokenType::HEX_LITERAL); // Already tested, but confirming
  EXPECT_EQ(ClassifyToken("0"),
            Lexer::TokenType::UNKNOWN); // Starts with 0 but not 0x
  EXPECT_EQ(ClassifyToken("0abc"),
            Lexer::TokenType::UNKNOWN); // Starts with 0 but not 0x
}

TEST_F(LexerTest, TokenizeLine_SingleToken) {
  // Test single keyword token
  auto tokens = TokenizeLine("let", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[0].line_, 0);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test single operator token
  tokens = TokenizeLine("+", 1);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::OPERATOR_PLUS);
  EXPECT_EQ(tokens[0].value_, "+");
  EXPECT_EQ(tokens[0].line_, 1);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test single hex literal
  tokens = TokenizeLine("0x1234", 2);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[0].value_, "0x1234");
  EXPECT_EQ(tokens[0].line_, 2);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test single param identifier
  tokens = TokenizeLine("_a", 3);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[0].value_, "_a");
  EXPECT_EQ(tokens[0].line_, 3);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test single unknown token
  tokens = TokenizeLine("abc", 4);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::UNKNOWN);
  EXPECT_EQ(tokens[0].value_, "abc");
  EXPECT_EQ(tokens[0].line_, 4);
  EXPECT_EQ(tokens[0].column_, 0);
}

TEST_F(LexerTest, TokenizeLine_CommentHandling) {
  // Test comment at start of line
  auto tokens = TokenizeLine("// comment", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "// comment");
  EXPECT_EQ(tokens[0].line_, 0);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test comment with text after
  tokens = TokenizeLine("// This is a comment", 1);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "// This is a comment");
  EXPECT_EQ(tokens[0].line_, 1);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test comment with multiple words
  tokens = TokenizeLine("// comment with multiple words", 2);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "// comment with multiple words");
  EXPECT_EQ(tokens[0].line_, 2);
  EXPECT_EQ(tokens[0].column_, 0);

  // Test comment with leading whitespace
  tokens = TokenizeLine("  // indented comment", 3);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "// indented comment");
  EXPECT_EQ(tokens[0].line_, 3);
  EXPECT_EQ(tokens[0].column_, 2);

  // Test comment only (no text after)
  tokens = TokenizeLine("//", 4);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "//");
  EXPECT_EQ(tokens[0].line_, 4);
  EXPECT_EQ(tokens[0].column_, 0);
}

TEST_F(LexerTest, TokenizeLine_EmptyAndWhitespace) {
  // Test empty line
  auto tokens = TokenizeLine("", 0);
  EXPECT_EQ(tokens.size(), 0);

  // Test line with only whitespace
  tokens = TokenizeLine("   ", 1);
  EXPECT_EQ(tokens.size(), 0);

  tokens = TokenizeLine("\t\t", 2);
  EXPECT_EQ(tokens.size(), 0);
}

TEST_F(LexerTest, TokenizeLine_ColumnNumberTracking) {
  // Test column number for single token
  auto tokens = TokenizeLine("let", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].column_, 0);

  // Note: Current implementation returns after first token,
  // so column tracking for multiple tokens is limited
  // Testing what the current implementation does
  tokens = TokenizeLine("let A", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[0].column_, 0);
}

TEST_F(LexerTest, TokenizeLine_LineNumberTracking) {
  // Test that line numbers are correctly passed through
  auto tokens = TokenizeLine("let", 5);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].line_, 5);

  tokens = TokenizeLine("def", 10);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].line_, 10);
}

TEST_F(LexerTest, TokenizeLine_VariousTokenTypes) {
  // Test keyword
  auto tokens = TokenizeLine("main", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_MAIN);

  // Test operator
  tokens = TokenizeLine("-", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::OPERATOR_MINUS);

  // Test multiply operator
  tokens = TokenizeLine("*", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::OPERATOR_MULTIPLY);

  // Test hex literal
  tokens = TokenizeLine("0xABCD", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[0].value_, "0xABCD");

  // Test param identifier
  tokens = TokenizeLine("_param", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[0].value_, "_param");
}

} // namespace
} // namespace boyo
