#pragma once

#include <stdexcept>
#include <string>

#include "frontend/ast.hpp"

// Semantic analysis errors
class SemaError : public std::runtime_error {
public:
  const Span span;
  const size_t line;

  SemaError(const Span& span_, const std::string& message, size_t line_)
      : std::runtime_error(message), span(span_), line(line_) {}
};

namespace sema {

// Perform semantic analysis on a program
// Throws SemaError if any semantic violations are found
void analyze(const Program& program);

} // namespace sema
