#pragma once

#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include <stdexcept>
#include <string>

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
std::string to_string(const LoxValue &value);
} // namespace evaluator
