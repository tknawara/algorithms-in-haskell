#pragma once

#include "frontend/ast.hpp"
#include "lexer.hpp"
#include <initializer_list>
#include <stdexcept>
#include <vector>

enum class precedence {
  none = 0,
  assignment = 10, // =
  ternary = 20,    // ? :
  logic_or = 30,   // or
  logic_and = 40,  // and
  equality = 50,   // == !=
  comparison = 60, // < > <= >=
  term = 70,       // + -
  factor = 80,     // * /
  unary = 90,      // ! -
  primary = 100
};

class ParseError : public std::runtime_error {
public:
  explicit ParseError(const std::string &message)
      : std::runtime_error(message) {}
};

class Parser {
private:
  const std::vector<Token> &tokens;
  const SourceContext &ctx;
  int current = 0;

public:
  Parser(const std::vector<Token> &tokens, const SourceContext &context);

  Expr parse_expression_only();
  Program parse();

private:
  Stmt parse_statement();
  Stmt parse_print_statement();
  Stmt parse_var_declaration_statement();
  Stmt parse_expression_statement();
  Stmt parse_block_statement();

  Expr parse_expression(int min_precedence = 0);
  Expr parse_prefix();
  int get_precedence(TokenType type) const;

  bool is_at_end() const;
  Token peek() const;
  Token previous() const;
  Token advance();

  bool check(TokenType type) const;
  bool match(std::initializer_list<TokenType> types);
  Token consume(TokenType type, const std::string &message);

  ParseError error(const Token &token, const std::string &message);
  void synchronize();
};