#include "evaluator/evaluator.hpp"

#include <iostream>

#include "core/source_context.hpp"

namespace evaluator {

bool is_truthy(const LoxValue &value) {
  if (std::holds_alternative<std::monostate>(value)) {
    return false;
  }
  if (std::holds_alternative<bool>(value)) {
    return std::get<bool>(value);
  }
  return true;
}

double check_number_operand(const Token &op, const LoxValue &operand) {
  if (std::holds_alternative<double>(operand)) {
    return std::get<double>(operand);
  }
  throw RuntimeError(op, "Operand must be a number.");
}

// ============================================================================
// Expression Evaluation
// ============================================================================

struct EvaluatorVisitor {
  Environment &env;
  const SourceContext &ctx;

  EvaluatorVisitor(Environment &environment, const SourceContext &context)
      : env(environment), ctx(context) {}

  LoxValue operator()(const Literal &expr) const { return expr.value; }

  LoxValue operator()(const Grouping &expr) const {
    return evaluate(*expr.expression, env, ctx);
  }

  LoxValue operator()(const Unary &expr) const {
    LoxValue right = evaluate(*expr.right, env, ctx);

    switch (expr.op.type) {
    case TokenType::bang:
      return !is_truthy(right);
    case TokenType::minus:
      return -check_number_operand(expr.op, right);
    default:
      return std::monostate{}; // Unreachable
    }
  }

  LoxValue operator()(const Binary &expr) const {
    LoxValue left = evaluate(*expr.left, env, ctx);

    switch (expr.op.type) {
    case TokenType::and_token: {
      if (!is_truthy(left))
        return left;
      return evaluate(*expr.right, env, ctx);
    } break;
    case TokenType::or_token: {
      if (is_truthy(left))
        return left;
      return evaluate(*expr.right, env, ctx);
    } break;
    default:
      break; // Fall through to normal evaluation
    }

    LoxValue right = evaluate(*expr.right, env, ctx);

    switch (expr.op.type) {
    case TokenType::minus: {
      return check_number_operand(expr.op, left) -
             check_number_operand(expr.op, right);
    } break;
    case TokenType::slash: {
      auto lhs = check_number_operand(expr.op, left);
      auto rhs = check_number_operand(expr.op, right);
      if (rhs == 0) {
        throw RuntimeError(expr.op, "division by zero!");
      }
      return lhs / rhs;
    } break;
    case TokenType::star: {
      return check_number_operand(expr.op, left) *
             check_number_operand(expr.op, right);
    } break;
    case TokenType::plus: {
      // Plus is overloaded in Lox: it adds numbers AND concatenates strings
      if (std::holds_alternative<double>(left) &&
          std::holds_alternative<double>(right)) {
        return std::get<double>(left) + std::get<double>(right);
      }
      if (std::holds_alternative<std::string>(left) &&
          std::holds_alternative<std::string>(right)) {
        return std::get<std::string>(left) + std::get<std::string>(right);
      }
      throw RuntimeError(expr.op,
                         "Operands must be two numbers or two strings.");
    } break;
    case TokenType::greater: {
      return check_number_operand(expr.op, left) >
             check_number_operand(expr.op, right);
    } break;
    case TokenType::greater_equal: {
      return check_number_operand(expr.op, left) >=
             check_number_operand(expr.op, right);
    } break;
    case TokenType::less: {
      return check_number_operand(expr.op, left) <
             check_number_operand(expr.op, right);
    } break;
    case TokenType::less_equal: {
      return check_number_operand(expr.op, left) <=
             check_number_operand(expr.op, right);
    } break;
    case TokenType::equal_equal: {
      return left == right;
    } break;
    case TokenType::bang_equal: {
      return left != right;
    } break;
    default:
      return std::monostate{}; // Unreachable
    }
  }

  LoxValue operator()(const Variable &expr) const {
    // This should not be called directly - evaluate() handles Variable
    // specially
    return std::monostate{};
  }

  LoxValue operator()(const Assign &expr) const {
    // This should not be called directly - evaluate() handles Assign specially
    return std::monostate{};
  }
};

LoxValue evaluate(const Expr &expr, Environment &env,
                  const SourceContext &ctx) {
  // Handle Variable specially
  if (std::holds_alternative<Variable>(expr.node)) {
    const auto &var = std::get<Variable>(expr.node);
    std::string name = std::string(var.name_token.get_lexeme(ctx));
    try {
      return env.get(name);
    } catch (const EnvironmentError &error) {
      // Re-throw as RuntimeError with proper token for line reporting
      throw RuntimeError(var.name_token, error.what());
    }
  }

  // Handle Assign specially
  if (std::holds_alternative<Assign>(expr.node)) {
    const auto &assign = std::get<Assign>(expr.node);
    // Extract name from source
    std::string name = std::string(assign.name_token.get_lexeme(ctx));
    // First evaluate the value
    LoxValue value = evaluate(*assign.value, env, ctx);
    // Then assign it
    try {
      env.assign(name, value);
      return value;
    } catch (const EnvironmentError &error) {
      // Re-throw as RuntimeError with proper token for line reporting
      throw RuntimeError(assign.name_token, error.what());
    }
  }

  return std::visit(EvaluatorVisitor(env, ctx), expr.node);
}

// Convenience overload
LoxValue evaluate(const Expr &expr, const SourceContext &ctx) {
  Environment env;
  return evaluate(expr, env, ctx);
}

// ============================================================================
// Statement Execution
// ============================================================================

// Helper to format a value for printing (without quotes for strings)
std::string stringify(const LoxValue &value) {
  if (std::holds_alternative<std::monostate>(value)) {
    return "nil";
  }
  if (std::holds_alternative<bool>(value)) {
    return std::get<bool>(value) ? "true" : "false";
  }
  if (std::holds_alternative<double>(value)) {
    // Format double without unnecessary decimals
    std::string result = std::to_string(std::get<double>(value));
    // Remove trailing zeros
    while (result.size() > 0 && result[result.size() - 1] == '0') {
      result.pop_back();
    }
    // Remove trailing decimal point if present
    if (result.size() > 0 && result[result.size() - 1] == '.') {
      result.pop_back();
    }
    return result;
  }
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  }
  return "unknown";
}

struct StatementExecutor {
  Environment &env;
  const SourceContext &ctx;

  StatementExecutor(Environment &environment, const SourceContext &context)
      : env(environment), ctx(context) {}

  void operator()(const ExpressionStmt &stmt) const {
    // Evaluate the expression and discard the result
    evaluate(*stmt.expression, env, ctx);
  }

  void operator()(const PrintStmt &stmt) const {
    LoxValue value = evaluate(*stmt.expression, env, ctx);
    std::cout << stringify(value) << "\n";
  }

  void operator()(const VarDeclaration &stmt) const {
    LoxValue value = std::monostate{}; // nil by default
    if (stmt.initializer.has_value()) {
      value = evaluate(*stmt.initializer.value(), env, ctx);
    }
    std::string name = std::string(stmt.name_token.get_lexeme(ctx));
    env.define(name, value);
  }
};

void execute(const Stmt &stmt, Environment &env, const SourceContext &ctx) {
  std::visit(StatementExecutor(env, ctx), stmt.node);
}

void execute_program(const Program &program, Environment &env,
                     const SourceContext &ctx) {
  for (const auto &stmt : program) {
    execute(stmt, env, ctx);
  }
}

// Convenience function
void execute_program(const Program &program, const SourceContext &ctx) {
  Environment env;
  execute_program(program, env, ctx);
}

} // namespace evaluator
