#include "frontend/parser.hpp"
#include "core/error_reporter.hpp"

Parser::Parser(const std::vector<Token> &tokens, const SourceContext &context)
    : tokens(tokens), ctx(context) {}

Expr Parser::parse() {
  try {
    return parse_expression();
  } catch (const ParseError &error) {
    // If we completely fail to parse an expression, we return a null/monostate
    // literal just to give the compiler a valid AST node, even though execution
    // will halt later.
    return Expr(Literal(std::monostate{}));
  }
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

  while (!is_at_end() && get_precedence(peek().type) >= min_precedence) {
    Token op = advance();
    int op_precedence = get_precedence(op.type);

    int next_min_precedence =
        (op.type == TokenType::equal) ? op_precedence : op_precedence + 1;

    Expr rhs = parse_expression(next_min_precedence);

    lhs = Expr(Binary(std::make_unique<Expr>(std::move(lhs)), op,
                      std::make_unique<Expr>(std::move(rhs))));
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