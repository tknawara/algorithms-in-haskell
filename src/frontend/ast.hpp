#pragma once

#include "frontend/lexer.hpp"
#include <memory>
#include <optional>
#include <variant>
#include <vector>

// Forward declarations
struct Expr;
struct Stmt;

// ============================================================================
// Expression Types
// ============================================================================

struct Binary {
  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;

  Binary(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r);
  ~Binary();

  Binary(Binary &&) = default;
  Binary &operator=(Binary &&) = default;
};

struct Unary {
  Token op;
  std::unique_ptr<Expr> right;

  Unary(Token o, std::unique_ptr<Expr> r);
  ~Unary();

  Unary(Unary &&) = default;
  Unary &operator=(Unary &&) = default;
};

struct Literal {
  LiteralType value;

  explicit Literal(LiteralType v);
  ~Literal();

  Literal(Literal &&) = default;
  Literal &operator=(Literal &&) = default;
};

struct Grouping {
  std::unique_ptr<Expr> expression;

  explicit Grouping(std::unique_ptr<Expr> expr);
  ~Grouping();

  Grouping(Grouping &&) = default;
  Grouping &operator=(Grouping &&) = default;
};

// Variable reference expression: just the token, name extracted from source
// during eval
struct Variable {
  Token name_token;

  explicit Variable(Token t);
  ~Variable();

  Variable(Variable &&) = default;
  Variable &operator=(Variable &&) = default;
};

// Assignment expression: target = value
// Example: a = 1, or a = b = 1 (right-associative)
struct Assign {
  Token name_token;
  std::unique_ptr<Expr> value;

  Assign(Token t, std::unique_ptr<Expr> v);
  ~Assign();

  Assign(Assign &&) = default;
  Assign &operator=(Assign &&) = default;
};

struct Expr {
  std::variant<Binary, Unary, Literal, Grouping, Variable, Assign> node;

  template <typename T> Expr(T &&n) : node(std::forward<T>(n)) {}
  Expr(Expr &&) = default;
  Expr &operator=(Expr &&) = default;
};

// Expression statement: an expression followed by a semicolon
struct ExpressionStmt {
  std::unique_ptr<Expr> expression;

  explicit ExpressionStmt(std::unique_ptr<Expr> expr);
  ~ExpressionStmt();

  ExpressionStmt(ExpressionStmt &&) = default;
  ExpressionStmt &operator=(ExpressionStmt &&) = default;
};

struct PrintStmt {
  std::unique_ptr<Expr> expression;

  explicit PrintStmt(std::unique_ptr<Expr> expr);
  ~PrintStmt();

  PrintStmt(PrintStmt &&) = default;
  PrintStmt &operator=(PrintStmt &&) = default;
};

struct VarDeclaration {
  Token name_token;
  std::optional<std::unique_ptr<Expr>> initializer;

  VarDeclaration(Token t, std::optional<std::unique_ptr<Expr>> init);
  ~VarDeclaration();

  VarDeclaration(VarDeclaration &&) = default;
  VarDeclaration &operator=(VarDeclaration &&) = default;
};

// Block statement: { statement1; statement2; ... }
// Creates a new scope for variable declarations
struct BlockStmt {
  std::vector<Stmt> statements;

  explicit BlockStmt(std::vector<Stmt> stmts);
  ~BlockStmt();

  BlockStmt(BlockStmt &&) = default;
  BlockStmt &operator=(BlockStmt &&) = default;
};

struct IfStmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;
  std::optional<std::unique_ptr<Stmt>> else_stmt;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body,
         std::optional<std::unique_ptr<Stmt>> else_stmt);
  ~IfStmt();

  IfStmt(IfStmt &&) = default;
  IfStmt &operator=(IfStmt &&) = default;
};

struct Stmt {
  std::variant<ExpressionStmt, PrintStmt, VarDeclaration, BlockStmt, IfStmt>
      node;

  template <typename T> Stmt(T &&n) : node(std::forward<T>(n)) {}
  Stmt(Stmt &&) = default;
  Stmt &operator=(Stmt &&) = default;
};

using Program = std::vector<Stmt>;