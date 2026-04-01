#pragma once

#include <stdexcept>
#include <string>

#include "frontend/lexer.hpp"

class RuntimeError : public std::runtime_error {
public:
  const Token &token;
  RuntimeError(const Token &token, const std::string &message)
      : std::runtime_error(message), token(token) {}
};