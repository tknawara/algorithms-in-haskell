#include "frontend/ast.hpp"
#include <memory>

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

Variable::Variable(Token t) : name_token(t) {}
Variable::~Variable() = default;

Assign::Assign(Token t, std::unique_ptr<Expr> v)
    : name_token(t), value(std::move(v)) {}
Assign::~Assign() = default;

FnCall::FnCall(Token name_token, std::vector<std::unique_ptr<Expr>> args)
    : name_token(std::move(name_token)), args(std::move(args)) {}
FnCall::~FnCall() = default;

// ============================================================================
// Statement Type Implementations
// ============================================================================

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
ExpressionStmt::~ExpressionStmt() = default;

PrintStmt::PrintStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}
PrintStmt::~PrintStmt() = default;

VarDeclaration::VarDeclaration(Token t,
                               std::optional<std::unique_ptr<Expr>> init)
    : name_token(t), initializer(std::move(init)) {}
VarDeclaration::~VarDeclaration() = default;

BlockStmt::BlockStmt(std::vector<Stmt> stmts) : statements(std::move(stmts)) {}
BlockStmt::~BlockStmt() = default;

IfStmt::IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body,
               std::optional<std::unique_ptr<Stmt>> else_stmt)
    : condition(std::move(condition)), body(std::move(body)),
      else_stmt(std::move(else_stmt)) {}
IfStmt::~IfStmt() = default;

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition,
                     std::unique_ptr<Stmt> body)
    : condition(std::move(condition)), body(std::move(body)) {}
WhileStmt::~WhileStmt() = default;

ForStmt::ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> cond,
                 std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> b)
    : initializer(std::move(init)), condition(std::move(cond)),
      increment(std::move(inc)), body(std::move(b)) {}
ForStmt::~ForStmt() = default;