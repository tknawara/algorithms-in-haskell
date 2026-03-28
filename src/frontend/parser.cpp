#include "frontend/parser.hpp"
#include "core/error_reporter.hpp"
#include "frontend/ast.hpp"

Parser::Parser(const std::vector<Token> &tokens, const SourceContext &context)
    : tokens(tokens), ctx(context) {}

// Parse a single expression (for backward compatibility with evaluate command)
Expr Parser::parse_expression_only() {
  try {
    return parse_expression();
  } catch (const ParseError &error) {
    // If we completely fail to parse an expression, we return a null/monostate
    // literal just to give the compiler a valid AST node, even though execution
    // will halt later.
    return Expr(Literal(std::monostate{}));
  }
}

// Parse a full program (list of statements)
Program Parser::parse() {
  Program program;

  try {
    while (!is_at_end()) {
      program.push_back(parse_statement());
    }
  } catch (const ParseError &error) {
    // Error already reported, return what we have
    synchronize();
  }

  return program;
}

// ============================================================================
// Statement Parsing
// ============================================================================

Stmt Parser::parse_statement() {
  if (match({TokenType::var_token})) {
    return parse_var_declaration_statement();
  }

  if (match({TokenType::print_token})) {
    return parse_print_statement();
  }

  return parse_expression_statement();
}

Stmt Parser::parse_print_statement() {
  Expr value = parse_expression();
  consume(TokenType::semicolon, "Expect ';' after value.");
  return Stmt(PrintStmt(std::make_unique<Expr>(std::move(value))));
}

Stmt Parser::parse_var_declaration_statement() {
  Token name_token = consume(TokenType::identifier, "Expect variable name.");

  std::optional<std::unique_ptr<Expr>> initializer = std::nullopt;
  if (match({TokenType::equal})) {
    initializer = std::make_unique<Expr>(parse_expression());
  }

  consume(TokenType::semicolon, "Expect ';' after variable declaration.");
  return Stmt(VarDeclaration(name_token, std::move(initializer)));
}

Stmt Parser::parse_expression_statement() {
  Expr value = parse_expression();
  consume(TokenType::semicolon, "Expect ';' after expression.");
  return Stmt(ExpressionStmt(std::make_unique<Expr>(std::move(value))));
}

// --- Precedence Climbing Core ---

int Parser::get_precedence(TokenType type) const {
  switch (type) {
  case TokenType::equal:
    return static_cast<int>(precedence::assignment);
  case TokenType::or_token:
    return static_cast<int>(precedence::logic_or);
  case TokenType::and_token:
    return static_cast<int>(precedence::logic_and);
  case TokenType::bang_equal:
  case TokenType::equal_equal:
    return static_cast<int>(precedence::equality);
  case TokenType::greater:
  case TokenType::greater_equal:
  case TokenType::less:
  case TokenType::less_equal:
    return static_cast<int>(precedence::comparison);
  case TokenType::plus:
  case TokenType::minus:
    return static_cast<int>(precedence::term);
  case TokenType::star:
  case TokenType::slash:
    return static_cast<int>(precedence::factor);
  default:
    return static_cast<int>(precedence::none);
  }
}

Expr Parser::parse_expression(int min_precedence) {
  Expr lhs = parse_prefix();

  // Keep consuming operators while they have precedence >= the minimum.
  // We use >= to handle right-associative operators like assignment correctly.
  // Assignment: a = b = c should parse as a = (b = c), not (a = b) = c
  // We also require precedence > 0 to skip non-operators like semicolon.
  while (!is_at_end()) {
    int precedence = get_precedence(peek().type);
    if (precedence < min_precedence || precedence == 0) {
      break;
    }
    Token op = advance();
    int op_precedence = get_precedence(op.type);

    // Handle assignment specially - it must target a variable
    if (op.type == TokenType::equal) {
      // Check that LHS is a valid assignment target
      if (!std::holds_alternative<Variable>(lhs.node)) {
        throw error(op, "Invalid assignment target.");
      }

      // Assignment is right-associative: a = b = c means a = (b = c)
      // Use precedence (not precedence + 1) for right-associativity
      Expr rhs = parse_expression(op_precedence);

      // Extract the variable token from lhs
      auto &var = std::get<Variable>(lhs.node);
      lhs = Expr(Assign(var.name_token, std::make_unique<Expr>(std::move(rhs))));
    } else {
      int next_min_precedence = op_precedence + 1;
      Expr rhs = parse_expression(next_min_precedence);
      lhs = Expr(Binary(std::make_unique<Expr>(std::move(lhs)), op,
                        std::make_unique<Expr>(std::move(rhs))));
    }
  }

  return lhs;
}

Expr Parser::parse_prefix() {
  if (match({TokenType::bang, TokenType::minus})) {
    Token op = previous();
    Expr right = parse_expression(static_cast<int>(precedence::unary));
    return Expr(Unary(op, std::make_unique<Expr>(std::move(right))));
  }

  if (match({TokenType::left_paren})) {
    Expr expr = parse_expression(static_cast<int>(precedence::assignment));
    consume(TokenType::right_paren, "Expect ')' after expression.");
    return Expr(Grouping(std::make_unique<Expr>(std::move(expr))));
  }

  if (match({TokenType::false_token}))
    return Expr(Literal(false));
  if (match({TokenType::true_token}))
    return Expr(Literal(true));
  if (match({TokenType::nil_token}))
    return Expr(Literal(std::monostate{}));

  if (match({TokenType::number, TokenType::string_token})) {
    return Expr(Literal(previous().literal));
  }

  if (match({TokenType::identifier})) {
    Token name_token = previous();
    return Expr(Variable(name_token));
  }

  // If we don't match any valid prefix, the syntax is garbage
  throw error(peek(), "Expect expression.");
}

// --- Token Consumption Helpers ---

bool Parser::is_at_end() const { return peek().type == TokenType::eof; }

Token Parser::peek() const { return tokens[current]; }

Token Parser::previous() const { return tokens[current - 1]; }

Token Parser::advance() {
  if (!is_at_end())
    current++;
  return previous();
}

bool Parser::check(TokenType type) const {
  if (is_at_end())
    return false;
  return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

Token Parser::consume(TokenType type, const std::string &message) {
  if (check(type))
    return advance();
  throw error(peek(), message);
}

// --- Error Handling & Recovery ---

ParseError Parser::error(const Token &token, const std::string &message) {
  if (token.type == TokenType::eof) {
    ErrorReporter::report(token.span, message + " at end", token.line, ctx);
  } else {
    ErrorReporter::report(token.span, message, token.line, ctx);
  }
  return ParseError(message);
}

void Parser::synchronize() {
  advance();

  while (!is_at_end()) {
    if (previous().type == TokenType::semicolon)
      return;

    switch (peek().type) {
    case TokenType::class_token:
    case TokenType::fun:
    case TokenType::var_token:
    case TokenType::for_token:
    case TokenType::if_token:
    case TokenType::while_token:
    case TokenType::print_token:
    case TokenType::return_token:
      return;
    default:
      break;
    }
    advance();
  }
}