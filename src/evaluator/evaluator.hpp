#pragma once

#include <stdexcept>
#include <string>

#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"

using LoxValue = LiteralType;

class RuntimeError : public std::runtime_error {
public:
  const Token &token;
  RuntimeError(const Token &token, const std::string &message)
      : std::runtime_error(message), token(token) {}
};

class SourceContext;

namespace evaluator {

// Evaluate an expression and return its value
LoxValue evaluate(const Expr &expr);

// Execute a statement
void execute(const Stmt &stmt);

// Execute a program (list of statements)
void execute_program(const Program &program);

} // namespace evaluator
