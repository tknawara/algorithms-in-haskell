#pragma once

#include <string>

#include "evaluator/evaluator.hpp"
#include "frontend/lexer.hpp"

namespace format {

// Format a literal for token output (e.g., "1.0", "hello", "null")
std::string literal(const LiteralType &value);

// Format a runtime value for evaluation output (e.g., "3", "hello", "true")
// Note: numbers don't have trailing .0 (e.g., "3" not "3.0")
std::string value(const LoxValue &val);

} // namespace format
