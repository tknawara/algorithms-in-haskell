#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "core/source_manager.hpp"
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

struct Token {
  TokenType type;
  Span span;
  LiteralType literal;
  int line;

  Token(TokenType t, Span s, LiteralType l, int ln)
      : type(t), span(s), literal(l), line(ln) {}
};

class Lexer {
private:
  const SourceContext &ctx;

public:
  Lexer(const SourceContext &context);
  std::vector<Token> scan_tokens();
};
