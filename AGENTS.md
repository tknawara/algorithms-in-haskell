# Interpreter - Agent Documentation

## Quick Reference

```bash
# Build (local development)
cmake -B build-agent -S . && cmake --build ./build-agent

# Or use the dev script
./dev_run.sh run test.lox

# Compiled binary
./build-agent/interpreter
```

## Project Structure

```
src/
├── main.cpp              # Entry point, command dispatch
├── commands.cpp/hpp      # Command pipeline (lex -> parse -> sema -> eval/run)
├── core/                 # Core utilities
│   ├── error_reporter    # Error reporting system (singleton pattern)
│   ├── file_io           # File reading utilities
│   ├── format            # Value formatting for output
│   ├── source_context    # Source code management (owns the source string)
│   └── span              # Source location tracking (offset + length)
├── frontend/             # Frontend (lexer + parser + AST)
│   ├── lexer.cpp/hpp     # Regex-based tokenizer
│   ├── parser.cpp/hpp    # Recursive descent parser with precedence climbing
│   ├── ast.cpp/hpp       # AST node definitions (using std::variant)
│   └── ast_printer.cpp/hpp  # AST visualization (visitor pattern)
├── sema/                 # Semantic analysis (NEW)
│   ├── sema.cpp/hpp      # Semantic validation pass
└── evaluator/            # Tree-walk interpreter
    ├── evaluator.cpp/hpp # Expression evaluation + statement execution
    └── environment.hpp   # Variable storage with parent scope support
```

## Architecture Overview

### Pipeline Flow

```
Source File -> Lexer -> Parser -> Sema -> Evaluator
                |         |        |        |
             Tokens     AST     Checks   Execution
```

**Error handling at each stage:**
- Lexer: Reports errors via `ErrorReporter`, continues tokenizing
- Parser: Throws `ParseError`, caught by `synchronize()` for recovery
- Sema: Throws `SemaError`, caught in `Pipeline::analyze()`
- Evaluator: Throws `RuntimeError` with token info for line numbers

### 1. Lexer (`frontend/lexer.cpp`)

- Uses regex-based maximal munch tokenization
- Tokens: `TokenType`, `Span` (source location), `literal` (value)
- Keywords: `and`, `class`, `else`, `false`, `fun`, `for`, `if`, `nil`, `or`, `print`, `return`, `super`, `this`, `true`, `var`, `while`

**Token convenience methods:**
```cpp
// Get the lexeme text from source (returns string_view)
token.get_lexeme(ctx)

// Get string value (works for string literals and identifiers)
token.get_string_value(ctx)
```

### 2. AST (`frontend/ast.hpp`)

**Design:** Uses `std::variant` for type-safe sum types with `std::unique_ptr` for ownership.

**Expressions:**
- `Literal` - numbers, strings, booleans, nil (`std::monostate`)
- `Unary` - prefix operators (`!`, `-`)
- `Binary` - infix operators (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `and`, `or`)
- `Grouping` - parenthesized expressions
- `Variable` - variable reference (stores token, name extracted during eval)
- `Assign` - assignment expression (right-associative: `a = b = 1`)

**Statements:**
- `ExpressionStmt` - expression followed by semicolon (result discarded)
- `PrintStmt` - `print` keyword + expression + semicolon
- `VarDeclaration` - `var` + identifier + optional initializer + semicolon
- `BlockStmt` - `{ statement1; statement2; ... }` (creates new scope)
- `IfStmt` - `if (condition) body [else else_stmt]`
- `WhileStmt` - `while (condition) body`
- `ForStmt` - `for (initializer; condition; increment) body`
- `NoOpStmt` - Empty statement (placeholder)

**Important AST patterns:**
```cpp
// AST nodes store tokens (not extracted strings) to preserve source location
// Names are extracted during evaluation via token.get_lexeme(ctx)
struct Variable {
  Token name_token;  // Contains Span for error reporting
};
```

### 3. Parser (`frontend/parser.cpp`)

**Expression Parsing:**
- Uses precedence climbing (top-down operator precedence)
- Precedence levels (low to high): assignment, logic_or, logic_and, equality, comparison, term, factor, unary, primary
- **Critical:** The while loop uses `>` (not `>=`) to prevent consuming semicolons as operators

**Statement Parsing:**
- `parse()` - parses a full program (list of statements until EOF)
- `parse_statement()` - dispatches to specific statement parsers based on lookahead
- Each statement parser consumes its trailing semicolon

**Error Recovery:**
```cpp
void synchronize();  // Skip tokens until statement boundary
```

### 4. Semantic Analysis (`sema/sema.cpp`) - NEW

**Purpose:** Validation pass between parsing and execution.

**Current checks:**
1. Variable declarations cannot be direct bodies of:
   - `for` loops
   - `while` loops
   - `if` statements
   - `else` branches

**Design pattern:** Visitor struct with `operator()` overloads:
```cpp
struct SemaChecker {
  void operator()(const ForStmt& stmt) const {
    check_body_not_var_decl(*stmt.body, "for loop");
    analyze_stmt(*stmt.body);
    if (stmt.initializer) analyze_stmt(*stmt.initializer);
  }
  void operator()(const VarDeclaration&) const { /* nothing */ }
  // ... etc
};
```

**Error handling:**
```cpp
throw SemaError(var_decl.name_token.span, "message", var_decl.name_token.line);
```

### 5. Evaluator (`evaluator/evaluator.cpp`)

**Expression Evaluation:**
```cpp
LoxValue evaluate(const Expr& expr, Environment& env, const SourceContext& ctx);
```
- `LoxValue` = `std::variant<std::monostate, double, std::string, bool>`
- Truthiness: `nil` and `false` are falsy, everything else truthy
- Short-circuiting: `and`/`or` don't evaluate RHS if not needed
- Variable lookup: Extract name from token, look up in environment

**Statement Execution:**
```cpp
void execute(const Stmt& stmt, Environment& env, const SourceContext& ctx);
void execute_program(const Program& program, Environment& env, const SourceContext& ctx);
```

**Design pattern:** Visitor struct `StatementExecutor` with `operator()` overloads for each statement type.

### 6. Environment (`evaluator/environment.hpp`)

```cpp
class Environment {
  Environment* parent;  // nullptr for global scope
  std::unordered_map<std::string, LoxValue> values;
  
  void define(name, value);   // Create new variable
  LoxValue get(name);         // Lookup (throws EnvironmentError if undefined)
  void assign(name, value);   // Update existing (throws if undefined)
};
```

**Scope chain:** Each `BlockStmt` creates a new `Environment` with the current as parent:
```cpp
void operator()(const BlockStmt& stmt) const {
  Environment block_env(&env);  // Parent = current env
  for (const auto& s : stmt.statements) {
    execute(s, block_env, ctx);
  }
}
```

## Common Patterns

### Visitor Pattern with std::variant

```cpp
// Define a visitor struct with overloads for each alternative
struct MyVisitor {
  void operator()(const Binary& expr) { /* handle binary */ }
  void operator()(const Literal& expr) { /* handle literal */ }
  // ... etc for all alternatives
};

// Apply to variant
std::visit(MyVisitor{}, expr.node);
```

### Error Reporting

```cpp
// From anywhere
ErrorReporter::report(span, message, line, ctx);

// From a token (convenience)
ErrorReporter::report_token(token, message, ctx);

// Check if any errors occurred
if (ErrorReporter::had_error) { /* handle */ }
```

### Token to String

```cpp
// During evaluation (when you have ctx)
std::string name = std::string(token.get_lexeme(ctx));
```

## Build Scripts

| Script | Purpose | Build Dir |
|--------|---------|-----------|
| `your_program.sh` | Official (Docker/codecrafters) | `build/` |
| `dev_run.sh` | Local development | `build-agent/` |

## Exit Codes

- `0` - Success
- `1` - Unknown command
- `65` - Compile error (parse/sema/lex errors)
- `70` - Runtime error

## Adding New Features

### New Statement Type

1. **AST** (`ast.hpp`): Add struct and add to `Stmt` variant
2. **AST** (`ast.cpp`): Add constructor/destructor implementation  
3. **Parser** (`parser.hpp`): Add `parse_X_statement()` declaration
4. **Parser** (`parser.cpp`): Implement parser + add to `parse_statement()` dispatch
5. **Sema** (`sema.cpp`): Add `operator()(const XStmt&)` to `SemaChecker`
6. **Evaluator** (`evaluator.cpp`): Add case to `StatementExecutor` visitor

### New Semantic Check

1. Add check to appropriate `operator()` in `SemaChecker`
2. Throw `SemaError(span, message, line)` on violation
3. The check runs automatically via `std::visit(SemaChecker{}, stmt.node)`

## Gotchas

1. **Build folder confusion:** Use `build-agent` locally, `build` for Docker
2. **Semicolons:** Parser expects them after every statement
3. **Precedence climbing:** Loop condition is `>` not `>=`
4. **String handling:** `stringify()` removes quotes; `format::value()` keeps them
5. **Error recovery:** Parser uses `synchronize()` which can skip tokens aggressively
6. **Token members:** Tokens store `literal` (parsed value) separate from lexeme text
