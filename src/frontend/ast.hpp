#pragma once

#include "frontend/lexer.hpp"
#include <memory>
#include <variant>

struct Expr;

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