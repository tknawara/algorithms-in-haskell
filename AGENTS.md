# Interpreter - Agent Documentation

## Build Instructions

**Important**: Use the `build-agent` folder for building (not `build`), as this is the folder mounted in the Docker environment.

```bash
# Clean build
cd /home/tarek/workspace/cpp/codecrafters-interpreter-cpp
rm -rf build-agent
mkdir build-agent
cd build-agent
cmake ..
make -j4

# Or use the one-liner:
cmake -B build-agent -S . && cmake --build ./build-agent
```

The compiled binary will be at `./build-agent/interpreter`.

## Project Structure

```
src/
├── main.cpp              # Entry point, command dispatch
├── commands.cpp/hpp      # Command handlers (tokenize, parse, evaluate, run)
├── core/                 # Core utilities
│   ├── error_reporter    # Error reporting system
│   ├── file_io           # File reading utilities
│   ├── format            # Value formatting for output
│   ├── source_manager    # Source code management
│   └── span              # Source location tracking
├── frontend/             # Frontend (lexer + parser + AST)
│   ├── lexer.cpp/hpp     # Tokenizer
│   ├── parser.cpp/hpp    # Recursive descent parser
│   ├── ast.cpp/hpp       # AST node definitions
│   └── ast_printer.cpp/hpp  # AST visualization
└── evaluator/            # Tree-walk interpreter
    ├── evaluator.cpp/hpp # Expression evaluation + statement execution
```

## Architecture Overview

### 1. Lexer (`frontend/lexer.cpp`)
- Uses regex-based maximal munch tokenization
- Tokens: `TokenType`, `Span` (source location), `literal` (value)
- Keywords recognized: `and`, `class`, `else`, `false`, `fun`, `for`, `if`, `nil`, `or`, `print`, `return`, `super`, `this`, `true`, `var`, `while`

### 2. AST (`frontend/ast.hpp`)

**Expressions:**
- `Literal` - numbers, strings, booleans, nil
- `Unary` - prefix operators (`!`, `-`)
- `Binary` - infix operators (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `and`, `or`)
- `Grouping` - parenthesized expressions
- `Variable` - variable reference (by name)
- `Assign` - assignment expression (`a = b = 1`)

**Statements:**
- `ExpressionStmt` - expression followed by semicolon (result discarded)
- `PrintStmt` - `print` keyword + expression + semicolon
- `VarDeclaration` - `var` + identifier + optional initializer + semicolon
- `Program` - `std::vector<Stmt>` representing a full program

### 3. Parser (`frontend/parser.cpp`)

**Expression Parsing:**
- Uses precedence climbing (top-down operator precedence)
- Precedence levels (low to high): assignment, ternary, logic_or, logic_and, equality, comparison, term, factor, unary, primary
- **Bug fix note**: The while loop condition uses `>` (not `>=`) to prevent consuming semicolons as operators

**Statement Parsing:**
- `parse()` - parses a full program (list of statements until EOF)
- `parse_expression_only()` - parses a single expression (for backward compatibility)
- `parse_statement()` - dispatches to print or expression statement
- `parse_print_statement()` - handles `print <expr>;`
- `parse_expression_statement()` - handles `<expr>;`

### 4. Environment (`evaluator/environment.hpp`)

Stores variable bindings during execution:
- `define(name, value)` - creates/updates a variable
- `get(name)` - retrieves a variable value (throws EnvironmentError if undefined)
- `assign(name, value)` - updates an existing variable
- Currently single global scope (nested scopes to be added later)

### 5. Evaluator (`evaluator/evaluator.cpp`)

**Expression Evaluation:**
- `evaluate(const Expr&, Environment&)` - evaluates expressions and returns `LoxValue`
- `LoxValue` = `std::variant<std::monostate, double, std::string, bool>`
- Truthiness: `nil` and `false` are falsy, everything else is truthy
- Operators: standard arithmetic, comparison, logical (`and`/`or` with short-circuiting)
- Variable lookup: looks up name in environment

**Statement Execution:**
- `execute(const Stmt&, Environment&)` - executes a single statement
- `execute_program(const Program&, Environment&)` - executes a list of statements
- `VarDeclaration` - defines variable in environment (nil if no initializer)
- `stringify()` - formats values for printing (no quotes on strings)

### 5. Commands (`commands.cpp`)

**Pipeline:**
- `lex()` - runs lexer, stores tokens
- `parse_expression()` - parses single expression into `expr_ast`
- `parse_program()` - parses statements into `program`
- `eval()` - evaluates `expr_ast`
- `run()` - executes `program`

**Commands:**
- `tokenize` - prints tokens
- `parse` - prints AST (expression mode)
- `evaluate` - evaluates expression, prints result
- `run` - executes statements (new)

## Usage Examples

### Run mode (statements)
```bash
./your_program.sh run program.lox
```

Input (`program.lox`):
```lox
var x = 5;
var y = 10;
print x + y;

var msg = "hello";
print msg;

var z;
print z;  // nil
```

Output:
```
15
hello
nil
```

### Evaluate mode (single expression)
```bash
./your_program.sh evaluate expr.lox
```

Input (`expr.lox`):
```lox
1 + 2 * 3
```

Output:
```
7
```

## Adding New Statement Types

To add a new statement (e.g., `if` statement):

1. **AST** (`ast.hpp`): Add new struct (e.g., `IfStmt`) and add it to the `Stmt` variant
2. **AST** (`ast.cpp`): Add constructor/destructor implementation
3. **Parser** (`parser.hpp`): Add `parse_if_statement()` declaration
4. **Parser** (`parser.cpp`): 
   - Add `parse_if_statement()` implementation
   - Update `parse_statement()` to dispatch to it
5. **Evaluator** (`evaluator.hpp`): Update if needed
6. **Evaluator** (`evaluator.cpp`): 
   - Add case to `StatementExecutor` visitor
   - Implement the execution logic

## Token Convenience Methods

```cpp
// Get the lexeme text from source (returns string_view)
token.get_lexeme(ctx)

// Get string value (works for string literals and identifiers)
token.get_string_value(ctx)
```

## Error Reporting Convenience

```cpp
// Report error at a token's location (extracts span and line automatically)
ErrorReporter::report_token(token, message, ctx);
```

## Variable Implementation Details

**Variable Declaration:**
- `var x = 5;` - declares with initializer
- `var y;` - declares without initializer (nil)

**Variable Name Storage:**
- Parser extracts variable names using `token.get_lexeme(ctx)` from the source
- `Variable` AST node stores: `name` (string) + `name_token` (for error line numbers)
- `VarDeclaration` AST node stores: `name` (string) + `name_token` + `initializer`
- Identifier tokens themselves don't store the name in the literal field

**Variable Assignment:**
- Assignment is an expression: `a = 1` returns 1
- Chained assignment: `a = b = 1` means `a = (b = 1)`
- Right-associative parsing with precedence climbing
- `Assign` AST node stores: name, name_token, value expression
- Evaluator assigns to environment and returns the value
- Invalid assignment target (e.g., `1 = 2`) throws parse error

**Variable Lookup:**
- Evaluator looks up the name string in the Environment
- RuntimeError thrown with token info if undefined

**Future Extensions:**
- Block scopes
- Function scopes with closures

## Common Gotchas

1. **Build folder**: Always use `build-agent`, not `build`
2. **Semicolons**: The parser expects semicolons after every statement
3. **Precedence climbing**: The loop condition is `>` not `>=` to avoid consuming non-operators
4. **String handling**: `stringify()` removes quotes; `format::value()` keeps them
5. **Error recovery**: Parser uses `synchronize()` to skip to next statement boundary on error
