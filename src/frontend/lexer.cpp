#include "frontend/lexer.hpp"

#include <regex>

#include "core/error_reporter.hpp"
#include "core/source_manager.hpp"

Lexer::Lexer(const SourceContext &context) : ctx(context) {}

std::string format_token_type(TokenType type) {
  switch (type) {
  case TokenType::left_paren:
    return "LEFT_PAREN";
  case TokenType::right_paren:
    return "RIGHT_PAREN";
  case TokenType::left_brace:
    return "LEFT_BRACE";
  case TokenType::right_brace:
    return "RIGHT_BRACE";
  case TokenType::comma:
    return "COMMA";
  case TokenType::dot:
    return "DOT";
  case TokenType::minus:
    return "MINUS";
  case TokenType::plus:
    return "PLUS";
  case TokenType::semicolon:
    return "SEMICOLON";
  case TokenType::star:
    return "STAR";
  case TokenType::bang_equal:
    return "BANG_EQUAL";
  case TokenType::bang:
    return "BANG";
  case TokenType::equal_equal:
    return "EQUAL_EQUAL";
  case TokenType::equal:
    return "EQUAL";
  case TokenType::greater_equal:
    return "GREATER_EQUAL";
  case TokenType::greater:
    return "GREATER";
  case TokenType::less_equal:
    return "LESS_EQUAL";
  case TokenType::less:
    return "LESS";
  case TokenType::slash:
    return "SLASH";
  case TokenType::string_token:
    return "STRING";
  case TokenType::number:
    return "NUMBER";
  case TokenType::identifier:
    return "IDENTIFIER";

  // Keywords
  case TokenType::and_token:
    return "AND";
  case TokenType::class_token:
    return "CLASS";
  case TokenType::else_token:
    return "ELSE";
  case TokenType::false_token:
    return "FALSE";
  case TokenType::fun:
    return "FUN";
  case TokenType::for_token:
    return "FOR";
  case TokenType::if_token:
    return "IF";
  case TokenType::nil_token:
    return "NIL";
  case TokenType::or_token:
    return "OR";
  case TokenType::print_token:
    return "PRINT";
  case TokenType::return_token:
    return "RETURN";
  case TokenType::super_token:
    return "SUPER";
  case TokenType::this_token:
    return "THIS";
  case TokenType::true_token:
    return "TRUE";
  case TokenType::var_token:
    return "VAR";
  case TokenType::while_token:
    return "WHILE";
  case TokenType::eof:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}

std::vector<Token> Lexer::scan_tokens() {
  std::vector<Token> tokens;
  int current_index = 0;
  int current_line = 1;
  const std::string &source = ctx.source_code;

  static const std::vector<std::pair<std::regex, TokenType>> token_regexes = {
      {std::regex(R"(\()"), TokenType::left_paren},
      {std::regex(R"(\))"), TokenType::right_paren},
      {std::regex(R"(\{)"), TokenType::left_brace},
      {std::regex(R"(\})"), TokenType::right_brace},
      {std::regex(R"(,)"), TokenType::comma},
      {std::regex(R"(\.)"), TokenType::dot},
      {std::regex(R"(-)"), TokenType::minus},
      {std::regex(R"(\+)"), TokenType::plus},
      {std::regex(R"(;)"), TokenType::semicolon},
      {std::regex(R"(\*)"), TokenType::star},
      {std::regex(R"(!=)"), TokenType::bang_equal},
      {std::regex(R"(!)"), TokenType::bang},
      {std::regex(R"(==)"), TokenType::equal_equal},
      {std::regex(R"(=)"), TokenType::equal},
      {std::regex(R"(>=)"), TokenType::greater_equal},
      {std::regex(R"(>)"), TokenType::greater},
      {std::regex(R"(<=)"), TokenType::less_equal},
      {std::regex(R"(<)"), TokenType::less},
      {std::regex(R"(/)"), TokenType::slash},
      {std::regex(R"("[^"]*(?:"|$))"), TokenType::string_token},
      {std::regex(R"(\d+(\.\d+)?)"), TokenType::number},
      {std::regex(R"(and\b)"), TokenType::and_token},
      {std::regex(R"(class\b)"), TokenType::class_token},
      {std::regex(R"(else\b)"), TokenType::else_token},
      {std::regex(R"(false\b)"), TokenType::false_token},
      {std::regex(R"(fun\b)"), TokenType::fun},
      {std::regex(R"(for\b)"), TokenType::for_token},
      {std::regex(R"(if\b)"), TokenType::if_token},
      {std::regex(R"(nil\b)"), TokenType::nil_token},
      {std::regex(R"(or\b)"), TokenType::or_token},
      {std::regex(R"(print\b)"), TokenType::print_token},
      {std::regex(R"(return\b)"), TokenType::return_token},
      {std::regex(R"(super\b)"), TokenType::super_token},
      {std::regex(R"(this\b)"), TokenType::this_token},
      {std::regex(R"(true\b)"), TokenType::true_token},
      {std::regex(R"(var\b)"), TokenType::var_token},
      {std::regex(R"(while\b)"), TokenType::while_token},
      {std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"), TokenType::identifier}};

  static const std::regex whitespace_regex(R"([ \t\r\n]+)");
  static const std::regex comment_regex(R"(//[^\n]*)");

  while (current_index < source.length()) {
    auto start_it = source.begin() + current_index;
    std::smatch match;

    // 1. Skip Whitespace
    if (std::regex_search(start_it, source.end(), match, whitespace_regex,
                          std::regex_constants::match_continuous)) {
      std::string space = match.str();
      current_line += std::count(space.begin(), space.end(), '\n');
      current_index += match.length();
      continue;
    }

    // 2. Skip Comments
    if (std::regex_search(start_it, source.end(), match, comment_regex,
                          std::regex_constants::match_continuous)) {
      current_index += match.length();
      continue;
    }

    // 3. Maximal Munch Tokenization
    int longest_match_length = 0;
    TokenType matched_type;
    bool found_match = false;

    for (const auto &[regex, type] : token_regexes) {
      if (std::regex_search(start_it, source.end(), match, regex,
                            std::regex_constants::match_continuous)) {
        if (match.length() > longest_match_length) {
          longest_match_length = match.length();
          matched_type = type;
          found_match = true;
        }
      }
    }

    if (found_match) {
      int match_end = current_index + longest_match_length;
      Span token_span(ctx.id, current_index, match_end);
      std::string_view lexeme_text = ctx.get_lexeme(token_span);

      if (matched_type == TokenType::string_token) {
        current_line +=
            std::count(lexeme_text.begin(), lexeme_text.end(), '\n');
      }

      LiteralType literal_val = std::monostate{};
      bool is_valid = true;

      if (matched_type == TokenType::string_token) {
        if (lexeme_text.back() != '"') {
          ErrorReporter::report(token_span, "Unterminated string.",
                                current_line, ctx);
          is_valid = false;
        } else {
          literal_val =
              std::string(lexeme_text.substr(1, lexeme_text.length() - 2));
        }
      } else if (matched_type == TokenType::number) {
        literal_val = std::stod(std::string(lexeme_text));
      }

      if (is_valid) {
        tokens.emplace_back(matched_type, token_span, literal_val,
                            current_line);
      }
      current_index = match_end;
    } else {
      Span error_span(ctx.id, current_index, current_index + 1);
      ErrorReporter::report(error_span,
                            std::string("Unexpected character: ") +
                                source[current_index],
                            current_line, ctx);
      current_index++;
    }
  }

  tokens.emplace_back(TokenType::eof,
                      Span(ctx.id, source.length(), source.length()),
                      std::monostate{}, current_line);
  return tokens;
}