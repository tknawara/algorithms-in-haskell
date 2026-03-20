#include <fstream>
#include <iostream>
#include <sstream>

#include "core/error_reporter.hpp"
#include "frontend/ast.hpp"
#include "frontend/ast_printer.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"

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

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./your_program.sh tokenize <filename>\n";
    return 1;
  }

  std::string command = argv[1];
  std::string filename = argv[2];

  if (command == "tokenize") {

    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error reading file: " << filename << "\n";
      return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_contents = buffer.str();

    // 1. Setup the architecture
    SourceContext ctx(0, filename, file_contents);
    Lexer lexer(ctx);

    // 2. Scan
    std::vector<Token> tokens = lexer.scan_tokens();

    // 3. Print Output
    for (const auto &token : tokens) {
      std::string type_name = format_token_type(token.type);
      std::string lexeme_text = (token.type == TokenType::eof)
                                    ? ""
                                    : std::string(ctx.get_lexeme(token.span));
      std::string literal_text = "null";

      if (std::holds_alternative<std::string>(token.literal)) {
        literal_text = std::get<std::string>(token.literal);
      } else if (std::holds_alternative<double>(token.literal)) {
        double n = std::get<double>(token.literal);
        if (n == static_cast<long long>(n)) {
          literal_text = std::to_string(static_cast<long long>(n)) + ".0";
        } else {
          std::string str = std::to_string(n);
          str.erase(str.find_last_not_of('0') + 1, std::string::npos);
          if (str.back() == '.')
            str += "0";
          literal_text = str;
        }
      }

      std::cout << type_name << " " << lexeme_text << " " << literal_text
                << "\n";
    }
  } else if (command == "parse") {

    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error reading file: " << filename << "\n";
      return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_contents = buffer.str();
    SourceContext ctx(0, filename, file_contents);
    Lexer lexer(ctx);
    std::vector<Token> tokens = lexer.scan_tokens();

    // If the lexer failed, CodeCrafters expects exit code 65
    if (ErrorReporter::had_error)
      return 65;

    Parser parser(tokens, ctx);
    Expr expression = parser.parse();

    // If the parser failed, CodeCrafters expects exit code 65
    if (ErrorReporter::had_error)
      return 65;

    std::cout << ast_printer::print(expression, ctx) << "\n";
    return 0;
  } else {
    std::cerr << "Unknown command: " << command << "\n";
    return 1;
  }

  return ErrorReporter::had_error ? 65 : 0;
}
