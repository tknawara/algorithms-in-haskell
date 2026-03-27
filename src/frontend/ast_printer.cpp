#include "ast_printer.hpp"

namespace ast_printer {

struct AstVisitor {
  const SourceContext &ctx;

  explicit AstVisitor(const SourceContext &c) : ctx(c) {}

  std::string operator()(const Binary &b) const {
    std::string op_str(ctx.get_lexeme(b.op.span));
    return "(" + op_str + " " + print(*b.left, ctx) + " " +
           print(*b.right, ctx) + ")";
  }

  std::string operator()(const Unary &u) const {
    std::string op_str(ctx.get_lexeme(u.op.span));
    return "(" + op_str + " " + print(*u.right, ctx) + ")";
  }

  std::string operator()(const Grouping &g) const {
    return "(group " + print(*g.expression, ctx) + ")";
  }

  std::string operator()(const Literal &l) const {
    if (std::holds_alternative<std::monostate>(l.value))
      return "nil";
    if (std::holds_alternative<bool>(l.value))
      return std::get<bool>(l.value) ? "true" : "false";
    if (std::holds_alternative<std::string>(l.value))
      return std::get<std::string>(l.value);

    if (std::holds_alternative<double>(l.value)) {
      double n = std::get<double>(l.value);
      if (n == static_cast<long long>(n))
        return std::to_string(static_cast<long long>(n)) + ".0";
      std::string str = std::to_string(n);
      str.erase(str.find_last_not_of('0') + 1, std::string::npos);
      if (str.back() == '.')
        str += "0";
      return str;
    }
    return "unknown";
  }
};

std::string print(const Expr &expr, const SourceContext &ctx) {
  return std::visit(AstVisitor(ctx), expr.node);
}

} // namespace ast_printer