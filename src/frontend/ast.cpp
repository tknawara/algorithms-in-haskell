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

Variable::Variable(std::string n, Token t) : name(std::move(n)), name_token(t) {}
Variable::~Variable() = default;

Assign::Assign(std::string n, Token t, std::unique_ptr<Expr> v)
    : name(std::move(n)), name_token(t), value(std::move(v)) {}
Assign::~Assign() = default;

// ============================================================================
// Statement Type Implementations
// ============================================================================

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
ExpressionStmt::~ExpressionStmt() = default;

PrintStmt::PrintStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
PrintStmt::~PrintStmt() = default;

VarDeclaration::VarDeclaration(std::string name, Token t,
                               std::optional<std::unique_ptr<Expr>> init)
    : name(std::move(name)), name_token(t), initializer(std::move(init)) {}
VarDeclaration::~VarDeclaration() = default;
