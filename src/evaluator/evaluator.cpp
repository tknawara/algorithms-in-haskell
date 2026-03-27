#include "evaluator.hpp"

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

struct EvaluatorVisitor {

  LoxValue operator()(const Literal &expr) const { return expr.value; }

  LoxValue operator()(const Grouping &expr) const {
    return evaluate(*expr.expression);
  }

  LoxValue operator()(const Unary &expr) const {
    LoxValue right = evaluate(*expr.right);

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
    LoxValue left = evaluate(*expr.left);
    LoxValue right = evaluate(*expr.right);

    switch (expr.op.type) {
    case TokenType::minus: {
      return check_number_operand(expr.op, left) -
             check_number_operand(expr.op, right);
    } break;
    case TokenType::slash: {
      auto lhs = check_number_operand(expr.op, left);
      auto rhs = check_number_operand(expr.op, right);
      if (rhs == 0) {
        throw new RuntimeError(expr.op, "division by zero!");
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

      // ... You'll add logic for >, <, ==, != here ...

    default:
      return std::monostate{}; // Unreachable
    }
  }
};

LoxValue evaluate(const Expr &expr) {
  return std::visit(EvaluatorVisitor(), expr.node);
}

std::string to_string(const LoxValue &value) {
  if (std::holds_alternative<std::monostate>(value))
    return "nil";
  if (std::holds_alternative<bool>(value))
    return std::get<bool>(value) ? "true" : "false";
  if (std::holds_alternative<std::string>(value))
    return std::get<std::string>(value);
  if (std::holds_alternative<double>(value)) {
    double n = std::get<double>(value);
    std::string text = std::to_string(n);
    // Strip trailing zeros for clean output
    text.erase(text.find_last_not_of('0') + 1, std::string::npos);
    if (text.back() == '.')
      text.pop_back();
    return text;
  }
  return "unknown";
}

} // namespace evaluator