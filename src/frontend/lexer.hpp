#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "core/source_context.hpp"
#include "core/span.hpp"

enum class TokenType {
  left_paren,
  right_paren,
  left_brace,
  right_brace,
  comma,
  dot,
  minus,
  plus,
  semicolon,
  slash,
  star,
  bang,
  bang_equal,
  equal,
  equal_equal,
  greater,
  greater_equal,
  less,
  less_equal,
  identifier,
  string_token,
  number,
  and_token,
  class_token,
  else_token,
  false_token,
  fun,
  for_token,
  if_token,
  nil_token,
  or_token,
  print_token,
  return_token,
  super_token,
  this_token,
  true_token,
  var_token,
  while_token,
  eof
};

using LiteralType = std::variant<std::monostate, double, std::string, bool>;

std::string format_token_type(TokenType type);

struct Token {
  TokenType type;
  Span span;
  LiteralType literal;
  int line;

  Token(TokenType t, Span s, LiteralType l, int ln)
      : type(t), span(s), literal(l), line(ln) {}

  // Get the lexeme (text) from the source context
  std::string_view get_lexeme(const SourceContext &ctx) const {
    return ctx.get_lexeme(span);
  }

  // Get the string value if this is an identifier/string token
  std::string get_string_value(const SourceContext &ctx) const {
    if (std::holds_alternative<std::string>(literal)) {
      return std::get<std::string>(literal);
    }
    // For identifiers, extract from source
    return std::string(get_lexeme(ctx));
  }
};

class Lexer {
private:
  const SourceContext &ctx;

public:
  Lexer(const SourceContext &context);
  std::vector<Token> scan_tokens();
};
