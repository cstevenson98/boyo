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
  EXPECT_EQ(ClassifyToken("print"), Lexer::TokenType::KEYWORD_PRINT);

  // Test symbols
  EXPECT_EQ(ClassifyToken("="), Lexer::TokenType::EQUALS);
  EXPECT_EQ(ClassifyToken("=>"), Lexer::TokenType::ARROW);

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

  // Test identifiers (user-defined names)
  EXPECT_EQ(ClassifyToken("abc"), Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(ClassifyToken("123"), Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(ClassifyToken("variable"), Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(ClassifyToken("("), Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(ClassifyToken(")"), Lexer::TokenType::IDENTIFIER);

  // Test edge cases - keywords must match exactly
  EXPECT_EQ(ClassifyToken("letx"),
            Lexer::TokenType::IDENTIFIER); // "let" prefix but not exact match
  EXPECT_EQ(ClassifyToken("0x"),
            Lexer::TokenType::HEX_LITERAL); // Already tested, but confirming
  EXPECT_EQ(ClassifyToken("0"),
            Lexer::TokenType::IDENTIFIER); // Starts with 0 but not 0x
  EXPECT_EQ(ClassifyToken("0abc"),
            Lexer::TokenType::IDENTIFIER); // Starts with 0 but not 0x
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

  // Test single identifier token
  tokens = TokenizeLine("abc", 4);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::IDENTIFIER);
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

  // Test inline comment (comment after other tokens)
  tokens = TokenizeLine("let A // comment", 5);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value_, "A");
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[2].value_, "// comment");
  EXPECT_EQ(tokens[2].line_, 5);
  EXPECT_EQ(tokens[2].column_, 6);

  // Test inline comment with multiple tokens before
  tokens = TokenizeLine("def double _a // function definition", 6);
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[3].value_, "// function definition");
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

  // Test column tracking for multiple tokens
  tokens = TokenizeLine("let A", 0);
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[0].column_, 0);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value_, "A");
  EXPECT_EQ(tokens[1].column_, 4); // "let " is 4 characters

  // Test with more tokens
  tokens = TokenizeLine("let A 0x10", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].column_, 0); // "let" at position 0
  EXPECT_EQ(tokens[1].column_, 4); // "A" at position 4
  EXPECT_EQ(tokens[2].column_, 6); // "0x10" at position 6
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::HEX_LITERAL);
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

TEST_F(LexerTest, TokenizeLine_MultipleTokens) {
  // Test let statement with hex literal
  auto tokens = TokenizeLine("let A 0x10", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::HEX_LITERAL);

  // Test Polish notation expression: * 0x02 _a
  tokens = TokenizeLine("* 0x02 _a", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::OPERATOR_MULTIPLY);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::PARAM_IDENTIFIER);

  // Test def statement: def double _a
  tokens = TokenizeLine("def double _a", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::PARAM_IDENTIFIER);

  // Test multiple spaces between tokens
  tokens = TokenizeLine("let  A   0x10", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[1].value_, "A");
  EXPECT_EQ(tokens[2].value_, "0x10");
}

TEST_F(LexerTest, Tokenize_MultipleLines) {
  // Test the public Tokenize method with multiple lines
  std::vector<std::string> lines = {"let A 0x10", "def double _a",
                                    "// comment"};

  auto tokens = lexer->Tokenize(lines);

  // Line 0: let A 0x10
  ASSERT_GE(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].line_, 0);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].line_, 0);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[2].line_, 0);

  // Line 1: def double _a
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[3].line_, 1);
  EXPECT_EQ(tokens[4].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[4].line_, 1);
  EXPECT_EQ(tokens[5].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[5].line_, 1);

  // Line 2: // comment
  EXPECT_EQ(tokens[6].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[6].line_, 2);
}

TEST_F(LexerTest, Tokenize_CompleteProgram) {
  // Test tokenizing a complete Boyo program with all token types
  std::vector<std::string> lines = {
      "let A = 0x10", "print A", "def double _a => * 0x10 _a", "main double A"};

  auto tokens = lexer->Tokenize(lines);

  // Verify we have all tokens (4 + 2 + 7 + 3 = 16 tokens)
  ASSERT_EQ(tokens.size(), 16);

  // Line 0: let A = 0x10
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::EQUALS);
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[3].line_, 0);

  // Line 1: print A
  EXPECT_EQ(tokens[4].type_, Lexer::TokenType::KEYWORD_PRINT);
  EXPECT_EQ(tokens[5].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[5].line_, 1);

  // Line 2: def double _a => * 0x10 _a
  EXPECT_EQ(tokens[6].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[7].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[8].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[9].type_, Lexer::TokenType::ARROW);
  EXPECT_EQ(tokens[10].type_, Lexer::TokenType::OPERATOR_MULTIPLY);
  EXPECT_EQ(tokens[11].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[12].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[12].line_, 2);

  // Line 3: main double A
  EXPECT_EQ(tokens[13].type_, Lexer::TokenType::KEYWORD_MAIN);
  EXPECT_EQ(tokens[14].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[15].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[15].line_, 3);
}

TEST_F(LexerTest, Tokenize_EmptyAndMixedLines) {
  std::vector<std::string> lines = {"let A", "", "  ", "// comment", "def"};

  auto tokens = lexer->Tokenize(lines);

  // Should have tokens from lines 0, 3, and 4
  // Empty lines and whitespace-only lines produce no tokens
  ASSERT_GE(tokens.size(), 4);

  // First line tokens
  EXPECT_EQ(tokens[0].line_, 0);
  EXPECT_EQ(tokens[1].line_, 0);

  // Comment from line 3
  auto comment_iter =
      std::find_if(tokens.begin(), tokens.end(), [](const auto &t) {
        return t.type_ == Lexer::TokenType::COMMENT;
      });
  ASSERT_NE(comment_iter, tokens.end());
  EXPECT_EQ(comment_iter->line_, 3);
}

TEST_F(LexerTest, TokenizeLine_SymbolTokens) {
  // Test equals sign
  auto tokens = TokenizeLine("=", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::EQUALS);
  EXPECT_EQ(tokens[0].value_, "=");

  // Test arrow
  tokens = TokenizeLine("=>", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::ARROW);
  EXPECT_EQ(tokens[0].value_, "=>");

  // Test let statement with equals
  tokens = TokenizeLine("let A = 0x10", 0);
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::EQUALS);
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::HEX_LITERAL);

  // Test def statement with arrow
  tokens = TokenizeLine("def double _a =>", 0);
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::ARROW);

  // Test print keyword
  tokens = TokenizeLine("print A", 0);
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_PRINT);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
}

TEST_F(LexerTest, TokenizeLine_RealBoyoSyntax) {
  // Test complete let statement: let A = 0x10
  auto tokens = TokenizeLine("let A = 0x10", 0);
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_LET);
  EXPECT_EQ(tokens[0].value_, "let");
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value_, "A");
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::EQUALS);
  EXPECT_EQ(tokens[2].value_, "=");
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[3].value_, "0x10");

  // Test def with arrow and Polish notation: def double _a => * 0x10 _a
  tokens = TokenizeLine("def double _a => * 0x10 _a", 0);
  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_DEF);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[1].value_, "double");
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::PARAM_IDENTIFIER);
  EXPECT_EQ(tokens[2].value_, "_a");
  EXPECT_EQ(tokens[3].type_, Lexer::TokenType::ARROW);
  EXPECT_EQ(tokens[4].type_, Lexer::TokenType::OPERATOR_MULTIPLY);
  EXPECT_EQ(tokens[5].type_, Lexer::TokenType::HEX_LITERAL);
  EXPECT_EQ(tokens[6].type_, Lexer::TokenType::PARAM_IDENTIFIER);

  // Test print statement
  tokens = TokenizeLine("print A", 0);
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_PRINT);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);

  // Test main statement: main double A
  tokens = TokenizeLine("main double A", 0);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::KEYWORD_MAIN);
  EXPECT_EQ(tokens[1].type_, Lexer::TokenType::IDENTIFIER);
  EXPECT_EQ(tokens[2].type_, Lexer::TokenType::IDENTIFIER);
}

TEST_F(LexerTest, TokenizeLine_EdgeCases) {
  // Test case sensitivity of keywords
  auto tokens = TokenizeLine("Let", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_,
            Lexer::TokenType::IDENTIFIER); // Keywords are case-sensitive

  // Test hex with lowercase
  tokens = TokenizeLine("0xabcd", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::HEX_LITERAL);

  // Test param identifier with numbers
  tokens = TokenizeLine("_123abc", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::PARAM_IDENTIFIER);

  // Test multiple slashes in comment
  tokens = TokenizeLine("// comment // more", 0);
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type_, Lexer::TokenType::COMMENT);
  EXPECT_EQ(tokens[0].value_, "// comment // more");
}

} // namespace
} // namespace boyo
