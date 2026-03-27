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
LoxValue evaluate(const Expr &expr);
} // namespace evaluator
