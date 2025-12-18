# Boyo Transpiler: Core Parsing Architecture Implementation Plan
## Overview
Implement a proper lexer/parser architecture for the Boyo functional programming language transpiler. The language uses Polish (prefix) notation for operators and treats all data as byte vectors (std::vector<uint8_t>).
### Target Syntax Support

- `let A = 0x10                      // Register declaration with hex literal`
print A                           // Print register to stderr
def double _a => * 0x10 _a       // Function definition with Polish notation
main double A                     // Program entry point
### Architecture Overview
Clean separation of concerns:
Tokenization (Lexer) - String → Tokens with types
Expression Parsing - Tokens → Expression AST (including Polish notation)
Statement Construction - Token groups → Statement AST
Code Generation - AST → C++ code
### Implementation Phases
Phase 1: Token System Foundation
1.1 Create Token Definitions
New file: src/compiler/lexer/include/lexer/token.hpp Define:
enum class TokenType with values:
Keywords: KEYWORD_LET, KEYWORD_PRINT, KEYWORD_DEF, KEYWORD_MAIN
Identifiers: IDENTIFIER (variables/functions), PARAM_IDENTIFIER (function params starting with _)
Literals: HEX_LITERAL (0x10, 0x1234)
Operators: OPERATOR (+, -, *, /)
Symbols: EQUALS (=), ARROW (=>)
Special: COMMENT, END_OF_FILE, UNKNOWN
struct Token with fields:
TokenType type
std::string value (raw token text)
size_t line (1-indexed line number)
size_t column (1-indexed column number)
Helper methods: IsKeyword(), IsOperator(), IsIdentifier(), IsLiteral()
Type alias: using TokenList = std::vector<Token>;
1.2 Create Lexer Interface
New file: src/compiler/lexer/include/lexer/lexer.hpp Define class Lexer with:
Public method: TokenList Tokenize(const std::vector<std::string>& lines) const
Private helpers:
TokenList TokenizeLine(const std::string& line, size_t line_number) const
TokenType ClassifyToken(const std::string& token) const
bool IsHexLiteral(const std::string& token) const
bool IsOperator(const std::string& token) const
bool IsKeyword(const std::string& token) const
1.3 Implement Lexer
New file: src/compiler/lexer/lexer.cpp Implementation strategy:
Tokenize(): Iterate lines, call TokenizeLine() for each
TokenizeLine():
Handle comments specially (everything after "//" is single token)
Split by whitespace
Track column positions
Call ClassifyToken() for each piece
ClassifyToken(): Check in order:
Keywords ("let", "print", "def", "main")
Operators (+, -, *, /)
Symbols (=, =>)
Hex literals (starts with "0x")
Parameter identifiers (starts with "_")
Regular identifiers (starts with letter)
Default to UNKNOWN
Note: Hex literals identified here but parsed to bytes later in expression layer.
1.4 Unit Tests for Lexer
New file: src/unit_tests/lexer/lexer_tests.cpp Test cases:
Single line tokenization with various token types
Multi-line tokenization
Comment handling
Hex literal detection
Position tracking (line/column)
Edge cases (empty lines, trailing spaces)
Phase 2: Expression System Expansion
2.1 Add New Expression Classes
Modify: src/compiler/statement/include/statement/expression.hpp Add 6 new expression classes (keep existing Expression, FunctionHeadExpression, LiteralExpression):
HexLiteralExpression
Constructor: explicit HexLiteralExpression(const std::string& hex_string)
Parse hex string to std::vector<uint8_t> bytes_
Store original string for ToString()
Getter: const std::vector<uint8_t>& GetBytes() const
Parsing rule: "0x1234" → {0x12, 0x34} (byte pairs)
IdentifierExpression
Represents variable/function names (e.g., "A", "double")
Store std::string name_
Getter: const std::string& GetName() const
ParameterExpression
Represents function parameters (e.g., "_a", "_b")
Store std::string param_name_
Getter: const std::string& GetParamName() const
OperatorExpression
Represents Polish notation operator with operands
Store: std::string operator_, std::unique_ptr<Expression> left_, std::unique_ptr<Expression> right_
Constructor: OperatorExpression(const std::string& op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
Getters for all fields
Example AST for * 0x10 _a:

OperatorExpression("*")
├── left: HexLiteralExpression({0x10})
└── right: ParameterExpression("_a")
KeywordExpression
Represents keywords (let, print, def, main)
Store std::string keyword_
Used for statement construction
2.2 Implement Expression Classes
Modify: src/compiler/statement/expression.cpp Implementation details:
HexLiteralExpression constructor: Parse "0x1234" by extracting byte pairs (skip "0x", process 2 chars at a time)
OperatorExpression::ToString(): Return Polish notation string (e.g., "* 0x10 _a")
Add new functions:
Update CreateExpression to use Tokens:

std::unique_ptr<Expression> CreateExpression(const Token& token)
Switch on token.type
Create appropriate expression subclass
Throw error for standalone OPERATOR tokens (must use ParsePolishExpression)
New ParsePolishExpression function:

std::unique_ptr<Expression> ParsePolishExpression(const TokenList& tokens, size_t& index)
Recursive descent parser
If current token is operator: recursively parse left and right operands, return OperatorExpression
Otherwise: create leaf expression via CreateExpression
Handles nested operators naturally through recursion
Example: + * 0x10 _a 0x05 builds nested operator tree
2.3 Unit Tests for Expressions
Modify: src/unit_tests/expression/expression_tests.cpp Add test cases for:
Each new expression class creation and ToString()
HexLiteralExpression byte parsing (single byte, multi-byte, odd length handling)
ParsePolishExpression with simple operators
ParsePolishExpression with nested operators
Error cases (malformed hex, unexpected tokens)
Phase 3: Statement System Expansion
3.1 Add New Statement Classes
Modify: src/compiler/statement/include/statement/statement.hpp Add 3 new statement classes:
LetStatement
Constructor: LetStatement(std::string var_name, std::unique_ptr<Expression> value_expr)
Store: std::string var_name_, std::unique_ptr<Expression> value_expr_
GenerateCode(): std::vector<uint8_t> <var_name> = <value_expr_code>;
DefStatement
Constructor: DefStatement(std::string func_name, std::vector<std::string> params, std::unique_ptr<Expression> body_expr)
Store: function name, parameter list, body expression
GenerateCode(): Generate C++ function with signature and body
MainStatement
Constructor: MainStatement(std::string func_name, std::vector<std::string> args)
Store: function to call, argument names
GenerateCode(): Call function, print result to stdout
3.2 Implement Statement Classes
Modify: src/compiler/statement/statement.cpp Implementation details:
LetStatement code generation:

std::vector<uint8_t> A = {0x10};
DefStatement code generation:

std::vector<uint8_t> double(const std::vector<uint8_t>& _a) {
  return multiply_vectors({0x10}, _a);
}
MainStatement code generation:

auto result = double(A);
print_vector(std::cout, result);
Update PrintStatement:
Handle both LiteralExpression (string literals) and IdentifierExpression (registers)
For literals: std::cerr << "text" << std::endl;
For identifiers: print_vector(std::cerr, <var_name>);
3.3 Unit Tests for Statements
Modify: src/unit_tests/statement/statement_tests.cpp Add tests for:
LetStatement code generation with hex literals
DefStatement code generation with various parameter counts
MainStatement code generation
Updated PrintStatement with identifiers
Edge cases (empty params, complex expressions)
Phase 4: Parser Refactoring
4.1 Update Parser to Use Lexer
Modify: src/compiler/parser/parser.cpp Refactor Parser::Parse():
Create Lexer instance
Call lexer.Tokenize(lines) to get all tokens
Call GroupTokensByStatement() to group by line
For each token group:
Check first token type
Call appropriate Parse*Statement() method
Handle comments
Add private helper methods:
GroupTokensByStatement:

std::vector<TokenList> GroupTokensByStatement(const TokenList& tokens) const
Group tokens by line number
Return vector of TokenList (one per line)
ParseLetStatement:

std::unique_ptr<Statement> ParseLetStatement(const TokenList& tokens) const
Expected: let <identifier> = <expression>
Validate tokens[1] is IDENTIFIER
Validate tokens[2] is EQUALS
Parse tokens[3+] as expression using ParsePolishExpression
Return LetStatement
ParsePrintStatement:

std::unique_ptr<Statement> ParsePrintStatement(const TokenList& tokens) const
Expected: print <literal|identifier>
Create expression from tokens[1]
Return PrintStatement
ParseDefStatement:

std::unique_ptr<Statement> ParseDefStatement(const TokenList& tokens) const
Expected: def <func_name> <param>... => <expression>
Find ARROW token position
Extract function name (tokens[1])
Extract parameters (tokens[2] to arrow)
Parse body expression (after arrow)
Return DefStatement
ParseMainStatement:

std::unique_ptr<Statement> ParseMainStatement(const TokenList& tokens) const
Expected: main <func_name> <arg>...
Extract function name (tokens[1])
Extract arguments (tokens[2+])
Return MainStatement
4.2 Update Parser Header
Modify: src/compiler/parser/include/parser/parser.hpp Add private method declarations for all the parsing helpers above.
4.3 Update Parser Tests
Modify: src/unit_tests/parser/parser_tests.cpp Add integration tests:
Parse let statements
Parse def statements with Polish notation
Parse main statements
Parse mixed statement files
Error handling (invalid syntax)
Phase 5: Code Generation and Runtime
5.1 Add Runtime Helpers
New file: src/compiler/runtime/runtime_helpers.hpp Define helper functions (as inline or header-only):
print_vector:

void print_vector(std::ostream& os, const std::vector<uint8_t>& vec)
Print bytes in hex format: "0x10" or "0x100"
multiply_vectors:

std::vector<uint8_t> multiply_vectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b)
Start simple: treat as single-byte scalar multiply
Return single byte result
add_vectors, subtract_vectors:
Similar to multiply, start with simple scalar operations
Can enhance later for multi-byte arithmetic
5.2 Update Code Generation Template
Modify: src/compiler/compiler/compiler.cpp Update kMainFunctionSnippet:
Include runtime_helpers.hpp content inline
Separate template into sections:
Includes and helpers
{boyo_functions} - Function definitions (DefStatement)
{boyo_main} - Main execution (LetStatement, PrintStatement, MainStatement)
Update GenerateProgramCode():
Separate statements into two groups:
Global: DefStatement → goes in {boyo_functions}
Main: All others → goes in {boyo_main}
Generate code for each group
Substitute into template
5.3 Expression Code Generation
Add GenerateCode() method to Expression classes (optional for Phase 5, needed for advanced features):
HexLiteralExpression::GenerateCode(): Return {0x10, 0x23}
IdentifierExpression::GenerateCode(): Return variable name
OperatorExpression::GenerateCode(): Return operator_helper(left->GenerateCode(), right->GenerateCode())
5.4 Update Compiler Tests
Modify: src/unit_tests/compiler/compiler_tests.cpp Add end-to-end tests:
Compile and run goal.boyo
Verify output is "0x100"
Test individual statement types
Test error cases (undefined variables, etc.)
Critical Files Summary
New Files (6 files)
src/compiler/lexer/include/lexer/token.hpp - Token definitions
src/compiler/lexer/include/lexer/lexer.hpp - Lexer interface
src/compiler/lexer/lexer.cpp - Lexer implementation
src/unit_tests/lexer/lexer_tests.cpp - Lexer tests
src/compiler/runtime/runtime_helpers.hpp - Runtime helpers
src/compiler/lexer/CMakeLists.txt - Build config for lexer
Modified Files (7 files)
src/compiler/statement/include/statement/expression.hpp - Add 6 new expression classes
src/compiler/statement/expression.cpp - Implement new expressions, add ParsePolishExpression
src/compiler/statement/include/statement/statement.hpp - Add 3 new statement classes
src/compiler/statement/statement.cpp - Implement new statements, update PrintStatement
src/compiler/parser/include/parser/parser.hpp - Add parsing helper methods
src/compiler/parser/parser.cpp - Refactor to use Lexer, implement statement parsers
src/compiler/compiler/compiler.cpp - Update template, separate global/main code
Test Files (4 files)
src/unit_tests/lexer/lexer_tests.cpp - New
src/unit_tests/expression/expression_tests.cpp - Expand
src/unit_tests/statement/statement_tests.cpp - Expand
src/unit_tests/parser/parser_tests.cpp - Expand
Key Design Decisions
Minimal but Proper Lexer: Token struct with types, but not overly complex
Hex Literals: Identify at tokenization, parse to bytes at expression creation
Polish Notation: Recursive descent parser builds AST naturally
Registers: Always std::vector<uint8_t>, no type variants
Inline Evaluation: No intermediate assignments, expressions used directly
Separation of Concerns: Lexer → Parser → AST → Code Generator (clean pipeline)
Testing Strategy
Each phase should be fully tested before moving to next:
Test lexer independently with various input
Test expressions in isolation
Test statements in isolation
Test parser integration with lexer
Test end-to-end compilation of goal.boyo
Success Criteria
Implementation complete when:
All unit tests pass
goal.boyo compiles successfully
Running compiled goal.boyo outputs: 0x100
Code is well-structured with clear separation of concerns