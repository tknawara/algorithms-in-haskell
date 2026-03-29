#include "sema/sema.hpp"

#include <variant>

namespace sema {

// Forward declarations
static void analyze_stmt(const Stmt& stmt);
static void check_body_statement(const Stmt& body, const std::string& context);

// Check if a statement is a variable declaration
static bool is_var_declaration(const Stmt& stmt) {
  return std::holds_alternative<VarDeclaration>(stmt.node);
}

// Semantic checker visitor struct
struct SemaChecker {
  void operator()(const ExpressionStmt& /*stmt*/) const {
    // Nothing to check
  }

  void operator()(const PrintStmt& /*stmt*/) const {
    // Nothing to check
  }

  void operator()(const VarDeclaration& /*stmt*/) const {
    // Nothing to check at this level
  }

  void operator()(const NoOpStmt& /*stmt*/) const {
    // Nothing to check
  }

  void operator()(const BlockStmt& stmt) const {
    // Recurse into all statements in block
    for (const auto& inner : stmt.statements) {
      analyze_stmt(inner);
    }
  }

  void operator()(const IfStmt& stmt) const {
    // Check body is not a var declaration
    check_body_statement(*stmt.body, "if statement");
    // Recurse into body
    analyze_stmt(*stmt.body);
    // Check else branch if present
    if (stmt.else_stmt) {
      check_body_statement(*stmt.else_stmt->get(), "else branch");
      analyze_stmt(*stmt.else_stmt->get());
    }
  }

  void operator()(const WhileStmt& stmt) const {
    // Check body is not a var declaration
    check_body_statement(*stmt.body, "while loop");
    // Recurse into body
    analyze_stmt(*stmt.body);
  }

  void operator()(const ForStmt& stmt) const {
    // Check body is not a var declaration
    check_body_statement(*stmt.body, "for loop");
    // Recurse into body
    analyze_stmt(*stmt.body);
    // Check initializer if present (it's fine to be var decl, but recurse)
    if (stmt.initializer) {
      analyze_stmt(*stmt.initializer);
    }
  }
};

// Check that a body statement is not a variable declaration
static void check_body_statement(const Stmt& body, const std::string& context) {
  if (is_var_declaration(body)) {
    const auto& var_decl = std::get<VarDeclaration>(body.node);
    throw SemaError(var_decl.name_token.span,
                    "Variable declaration not allowed in " + context + 
                    " body. Wrap the declaration in a block.",
                    var_decl.name_token.line);
  }
}

// Analyze a single statement
static void analyze_stmt(const Stmt& stmt) {
  std::visit(SemaChecker{}, stmt.node);
}

void analyze(const Program& program) {
  for (const auto& stmt : program) {
    analyze_stmt(stmt);
  }
}

} // namespace sema
