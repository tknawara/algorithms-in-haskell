#pragma once

#include "frontend/ast.hpp"
#include <string>

class SourceContext;

namespace ast_printer {
std::string print(const Expr &expr, const SourceContext &ctx);
} // namespace ast_printer