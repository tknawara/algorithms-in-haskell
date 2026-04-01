#pragma once

#include "environment.hpp"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"

using LoxValue = LiteralType;

class SourceContext;

namespace evaluator {

// Evaluate an expression and return its value
LoxValue evaluate(const Expr &expr, Environment &env, const SourceContext &ctx);

// Convenience function that creates a temporary environment
LoxValue evaluate(const Expr &expr, const SourceContext &ctx);

// Execute a statement
void execute(const Stmt &stmt, Environment &env, const SourceContext &ctx);

// Execute a program (list of statements)
void execute_program(const Program &program, Environment &env,
                     const SourceContext &ctx);

// Convenience function that creates an environment
void execute_program(const Program &program, const SourceContext &ctx);

} // namespace evaluator
