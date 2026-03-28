#pragma once

#include "frontend/lexer.hpp"
#include <memory>
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

struct Expr {
  std::variant<Binary, Unary, Literal, Grouping> node;

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

struct Stmt {
  std::variant<ExpressionStmt, PrintStmt> node;

  template <typename T> Stmt(T &&n) : node(std::forward<T>(n)) {}
  Stmt(Stmt &&) = default;
  Stmt &operator=(Stmt &&) = default;
};

using Program = std::vector<Stmt>;