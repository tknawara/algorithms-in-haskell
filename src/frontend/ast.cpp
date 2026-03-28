#include "frontend/ast.hpp"

// ============================================================================
// Expression Type Implementations
// ============================================================================

Binary::Binary(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
    : left(std::move(l)), op(o), right(std::move(r)) {}
Binary::~Binary() = default;

Unary::Unary(Token o, std::unique_ptr<Expr> r) : op(o), right(std::move(r)) {}
Unary::~Unary() = default;

Literal::Literal(LiteralType v) : value(std::move(v)) {}
Literal::~Literal() = default;

Grouping::Grouping(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
Grouping::~Grouping() = default;

// ============================================================================
// Statement Type Implementations
// ============================================================================

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
ExpressionStmt::~ExpressionStmt() = default;

PrintStmt::PrintStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
PrintStmt::~PrintStmt() = default;