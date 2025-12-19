# Recursive Descent Parser for Polish Notation

## Overview

This document explains the recursive descent parser implementation for parsing Polish (prefix) notation expressions in the Boyo language. Polish notation is ideal for recursive descent parsing because it naturally maps to recursive function calls.

## What is a Recursive Descent Parser?

A recursive descent parser is a top-down parsing technique where:
- Each grammar rule corresponds to a function
- Functions call each other recursively to match the input
- Parsing proceeds from the root of the parse tree downward
- The parser "descends" through the grammar rules recursively

### Advantages for Polish Notation

Polish notation (prefix notation) is particularly well-suited for recursive descent because:
1. **Operator-first structure**: The operator comes before its operands, making it easy to identify what to parse next
2. **Natural recursion**: Each operator naturally requires parsing its operands, which may themselves be operators
3. **No ambiguity**: Unlike infix notation, no operator precedence rules are needed
4. **Left-to-right parsing**: Tokens are consumed left-to-right, matching how we read code

## Polish Notation Basics

### Syntax Rules

In Polish notation:
- Operators come first: `+ 5 3` means `5 + 3`
- Operands follow: `* 2 4` means `2 * 4`
- Nested expressions: `+ * 2 3 4` means `(2 * 3) + 4`

### Expression Grammar

```
expression := operator_expression | leaf_expression
operator_expression := OPERATOR expression expression
leaf_expression := HEX_LITERAL | IDENTIFIER | PARAM_IDENTIFIER
```

In plain English:
- An expression is either an operator with two sub-expressions, or a leaf (literal/identifier)
- An operator expression consists of an operator followed by two expressions
- A leaf expression is a hex literal, identifier, or parameter identifier

## Algorithm Design

### Core Function Signature

```cpp
std::unique_ptr<Expression> ParsePolishExpression(
    const TokenList& tokens, 
    size_t& index
)
```

**Parameters:**
- `tokens`: The list of all tokens to parse
- `index`: Current position in the token list (passed by reference to track position)

**Returns:**
- A unique pointer to the parsed `Expression` AST node

### Algorithm Pseudocode

```
function ParsePolishExpression(tokens, index):
    if index >= tokens.length:
        throw error("Unexpected end of expression")
    
    current_token = tokens[index]
    index = index + 1  // Consume the token
    
    if current_token.type == OPERATOR:
        // This is an operator expression
        operator = current_token.value
        
        // Recursively parse left operand
        left = ParsePolishExpression(tokens, index)
        
        // Recursively parse right operand
        right = ParsePolishExpression(tokens, index)
        
        return OperatorExpression(operator, left, right)
    
    else if current_token.type == HEX_LITERAL:
        return HexLiteralExpression(current_token.value)
    
    else if current_token.type == IDENTIFIER:
        return IdentifierExpression(current_token.value)
    
    else if current_token.type == PARAM_IDENTIFIER:
        return ParameterExpression(current_token.value)
    
    else:
        throw error("Unexpected token type: " + current_token.type)
```

### Key Design Decisions

1. **Index by Reference**: The `index` parameter is passed by reference so it advances as tokens are consumed. This allows the recursive calls to naturally consume tokens in order.

2. **Operator-First Recognition**: When we see an operator, we immediately know we need to parse two operands recursively.

3. **Leaf Node Handling**: Non-operator tokens are terminal nodes (leaves) that don't require further parsing.

4. **Natural Recursion**: The recursive structure of the parser mirrors the recursive structure of Polish notation expressions.

## Step-by-Step Examples

### Example 1: Simple Expression `* 0x10 _a`

**Input tokens:**
```
[OPERATOR("*"), HEX_LITERAL("0x10"), PARAM_IDENTIFIER("_a")]
```

**Parse trace:**

```
ParsePolishExpression(tokens, index=0)
├─ Current token: OPERATOR("*") at index 0
├─ Consume token, index → 1
├─ Parse left operand:
│  └─ ParsePolishExpression(tokens, index=1)
│     ├─ Current token: HEX_LITERAL("0x10") at index 1
│     ├─ Consume token, index → 2
│     └─ Return: HexLiteralExpression("0x10")
├─ Parse right operand:
│  └─ ParsePolishExpression(tokens, index=2)
│     ├─ Current token: PARAM_IDENTIFIER("_a") at index 2
│     ├─ Consume token, index → 3
│     └─ Return: ParameterExpression("_a")
└─ Return: OperatorExpression("*", HexLiteralExpression("0x10"), ParameterExpression("_a"))
```

**Result AST:**
```
OperatorExpression("*")
├── left: HexLiteralExpression({0x10})
└── right: ParameterExpression("_a")
```

### Example 2: Nested Expression `+ * 0x10 _a 0x05`

**Input tokens:**
```
[OPERATOR("+"), OPERATOR("*"), HEX_LITERAL("0x10"), 
 PARAM_IDENTIFIER("_a"), HEX_LITERAL("0x05")]
```

**Parse trace:**

```
ParsePolishExpression(tokens, index=0)
├─ Current token: OPERATOR("+") at index 0
├─ Consume token, index → 1
├─ Parse left operand:
│  └─ ParsePolishExpression(tokens, index=1)
│     ├─ Current token: OPERATOR("*") at index 1
│     ├─ Consume token, index → 2
│     ├─ Parse left operand:
│     │  └─ ParsePolishExpression(tokens, index=2)
│     │     ├─ Current token: HEX_LITERAL("0x10") at index 2
│     │     ├─ Consume token, index → 3
│     │     └─ Return: HexLiteralExpression("0x10")
│     ├─ Parse right operand:
│     │  └─ ParsePolishExpression(tokens, index=3)
│     │     ├─ Current token: PARAM_IDENTIFIER("_a") at index 3
│     │     ├─ Consume token, index → 4
│     │     └─ Return: ParameterExpression("_a")
│     └─ Return: OperatorExpression("*", HexLiteralExpression("0x10"), ParameterExpression("_a"))
├─ Parse right operand:
│  └─ ParsePolishExpression(tokens, index=4)
│     ├─ Current token: HEX_LITERAL("0x05") at index 4
│     ├─ Consume token, index → 5
│     └─ Return: HexLiteralExpression("0x05")
└─ Return: OperatorExpression("+", OperatorExpression("*", ...), HexLiteralExpression("0x05"))
```

**Result AST:**
```
OperatorExpression("+")
├── left: OperatorExpression("*")
│   ├── left: HexLiteralExpression({0x10})
│   └── right: ParameterExpression("_a")
└── right: HexLiteralExpression({0x05})
```

**Evaluation order:** `(* 0x10 _a) + 0x05` = `(0x10 * _a) + 0x05`

### Example 3: Deeply Nested `* + 1 2 * 3 4`

**Input tokens:**
```
[OPERATOR("*"), OPERATOR("+"), HEX_LITERAL("0x01"), HEX_LITERAL("0x02"),
 OPERATOR("*"), HEX_LITERAL("0x03"), HEX_LITERAL("0x04")]
```

**Result AST:**
```
OperatorExpression("*")
├── left: OperatorExpression("+")
│   ├── left: HexLiteralExpression({0x01})
│   └── right: HexLiteralExpression({0x02})
└── right: OperatorExpression("*")
    ├── left: HexLiteralExpression({0x03})
    └── right: HexLiteralExpression({0x04})
```

**Evaluation order:** `(1 + 2) * (3 * 4)` = `3 * 12` = `36`

## Implementation Details

### File Location

**File:** `src/compiler/statement/expression.cpp`

### Function Implementation

```cpp
std::unique_ptr<Expression> ParsePolishExpression(
    const TokenList& tokens, 
    size_t& index
) {
    // Check bounds
    if (index >= tokens.size()) {
        throw std::runtime_error(
            "Unexpected end of expression at line " + 
            std::to_string(tokens.empty() ? 0 : tokens.back().line)
        );
    }
    
    const Token& current = tokens[index];
    index++;  // Consume the token
    
    // Handle operator expressions
    if (current.type == TokenType::OPERATOR) {
        std::string op = current.value;
        
        // Recursively parse left operand
        auto left = ParsePolishExpression(tokens, index);
        
        // Recursively parse right operand
        auto right = ParsePolishExpression(tokens, index);
        
        return std::make_unique<OperatorExpression>(
            op, std::move(left), std::move(right)
        );
    }
    
    // Handle leaf expressions
    return CreateExpression(current);
}
```

### Helper Function: CreateExpression

```cpp
std::unique_ptr<Expression> CreateExpression(const Token& token) {
    switch (token.type) {
        case TokenType::HEX_LITERAL:
            return std::make_unique<HexLiteralExpression>(token.value);
        
        case TokenType::IDENTIFIER:
            return std::make_unique<IdentifierExpression>(token.value);
        
        case TokenType::PARAM_IDENTIFIER:
            return std::make_unique<ParameterExpression>(token.value);
        
        case TokenType::OPERATOR:
            throw std::runtime_error(
                "Standalone operator token not allowed. "
                "Use ParsePolishExpression for operator expressions."
            );
        
        default:
            throw std::runtime_error(
                "Unexpected token type: " + 
                std::to_string(static_cast<int>(token.type))
            );
    }
}
```

## Usage in Statement Parsing

### Integration with LetStatement

```cpp
std::unique_ptr<Statement> ParseLetStatement(const TokenList& tokens) const {
    // tokens[0] = KEYWORD_LET
    // tokens[1] = IDENTIFIER (variable name)
    // tokens[2] = EQUALS
    // tokens[3+] = expression tokens
    
    if (tokens.size() < 4) {
        throw std::runtime_error("Invalid let statement: too few tokens");
    }
    
    std::string var_name = tokens[1].value;
    size_t expr_start = 3;  // Start after "let", identifier, "="
    
    // Parse the expression using recursive descent
    size_t index = expr_start;
    auto expr = ParsePolishExpression(tokens, index);
    
    return std::make_unique<LetStatement>(var_name, std::move(expr));
}
```

### Integration with DefStatement

```cpp
std::unique_ptr<Statement> ParseDefStatement(const TokenList& tokens) const {
    // tokens[0] = KEYWORD_DEF
    // tokens[1] = IDENTIFIER (function name)
    // tokens[2..n-1] = PARAM_IDENTIFIER (parameters)
    // tokens[n] = ARROW
    // tokens[n+1..] = expression tokens (body)
    
    // Find arrow position
    size_t arrow_pos = 0;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == TokenType::ARROW) {
            arrow_pos = i;
            break;
        }
    }
    
    std::string func_name = tokens[1].value;
    std::vector<std::string> params;
    
    // Extract parameters (tokens[2] to tokens[arrow_pos-1])
    for (size_t i = 2; i < arrow_pos; i++) {
        params.push_back(tokens[i].value);
    }
    
    // Parse body expression
    size_t index = arrow_pos + 1;
    auto body = ParsePolishExpression(tokens, index);
    
    return std::make_unique<DefStatement>(
        func_name, params, std::move(body)
    );
}
```

## Error Handling

### Common Error Cases

1. **Unexpected End of Expression**
   - **Cause**: Not enough tokens to complete an operator expression
   - **Example**: `* 0x10` (missing right operand)
   - **Detection**: `index >= tokens.size()` when expecting more tokens
   - **Error Message**: "Unexpected end of expression at line X"

2. **Invalid Token Type**
   - **Cause**: Token type that can't be part of an expression
   - **Example**: `KEYWORD_LET` in expression position
   - **Detection**: Switch statement default case
   - **Error Message**: "Unexpected token type: X"

3. **Standalone Operator**
   - **Cause**: Operator token passed to `CreateExpression` instead of `ParsePolishExpression`
   - **Detection**: Explicit check in `CreateExpression`
   - **Error Message**: "Standalone operator token not allowed"

4. **Extra Tokens**
   - **Cause**: More tokens than needed for expression
   - **Example**: `* 0x10 _a 0x05` when only `* 0x10 _a` was expected
   - **Detection**: Check `index` after parsing to see if all tokens consumed
   - **Note**: May be valid if expression is part of larger statement

### Error Reporting

Include position information in error messages:
```cpp
throw std::runtime_error(
    "Unexpected token '" + token.value + 
    "' at line " + std::to_string(token.line) + 
    ", column " + std::to_string(token.column)
);
```

## Testing Strategy

### Unit Test Cases

1. **Simple Binary Operations**
   - `* 0x10 0x05` → OperatorExpression with two HexLiteralExpression children
   - `+ A B` → OperatorExpression with two IdentifierExpression children

2. **Nested Operations**
   - `+ * 0x10 _a 0x05` → Nested OperatorExpression
   - `* + 1 2 3` → OperatorExpression with nested left operand

3. **Deep Nesting**
   - `* + 1 2 * 3 4` → Multiple levels of nesting
   - `+ * 2 3 * 4 5` → Symmetric nesting

4. **Edge Cases**
   - Single token: `0x10` → HexLiteralExpression (no recursion)
   - Single identifier: `A` → IdentifierExpression (no recursion)
   - Single parameter: `_a` → ParameterExpression (no recursion)

5. **Error Cases**
   - Incomplete expression: `* 0x10` → Should throw
   - Invalid token: `let 0x10` → Should throw
   - Empty token list: `[]` → Should throw

### Test Implementation Example

```cpp
TEST(ParsePolishExpression, SimpleBinaryOperation) {
    TokenList tokens = {
        Token{TokenType::OPERATOR, "*", 1, 1},
        Token{TokenType::HEX_LITERAL, "0x10", 1, 3},
        Token{TokenType::HEX_LITERAL, "0x05", 1, 8}
    };
    
    size_t index = 0;
    auto expr = ParsePolishExpression(tokens, index);
    
    ASSERT_NE(expr, nullptr);
    auto* op_expr = dynamic_cast<OperatorExpression*>(expr.get());
    ASSERT_NE(op_expr, nullptr);
    EXPECT_EQ(op_expr->GetOperator(), "*");
    EXPECT_EQ(index, 3);  // All tokens consumed
}
```

## Performance Considerations

### Time Complexity
- **Best case**: O(n) where n is the number of tokens
- **Worst case**: O(n) - same, as we visit each token exactly once
- Each token is consumed exactly once, no backtracking needed

### Space Complexity
- **Stack depth**: O(d) where d is the maximum nesting depth
- **AST size**: O(n) where n is the number of tokens
- **Total**: O(n + d) = O(n) in typical cases

### Optimization Opportunities
1. **Token pre-validation**: Check token types before parsing
2. **Early termination**: Stop parsing if invalid token encountered
3. **Memory pooling**: Reuse expression objects if needed (premature optimization)

## Comparison with Other Parsing Approaches

### vs. Shunting Yard (Infix Notation)
- **Shunting Yard**: Requires operator precedence, parentheses handling
- **Recursive Descent (Polish)**: No precedence needed, simpler algorithm

### vs. LR Parser
- **LR Parser**: More general, can handle more complex grammars
- **Recursive Descent (Polish)**: Simpler, more readable, sufficient for Polish notation

### vs. Iterative Parsing
- **Iterative**: Would require explicit stack management
- **Recursive Descent**: Natural stack via function call stack, cleaner code

## Summary

The recursive descent parser for Polish notation is:
- **Simple**: Each operator naturally requires parsing two operands
- **Efficient**: O(n) time complexity, visits each token once
- **Natural**: The recursive structure mirrors the expression structure
- **Maintainable**: Easy to understand and modify
- **Robust**: Clear error handling at each step

The implementation follows a straightforward pattern:
1. Check if current token is an operator
2. If yes: recursively parse left and right operands
3. If no: create appropriate leaf expression
4. Return the constructed expression tree

This approach elegantly handles both simple and deeply nested expressions without any special cases or complex state management.

