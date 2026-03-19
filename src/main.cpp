#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "core/error_reporter.hpp"
#include "frontend/lexer.hpp"

std::string format_token_type(token_type type) {
  switch (type) {
  case token_type::left_paren:
    return "LEFT_PAREN";
  case token_type::right_paren:
    return "RIGHT_PAREN";
  case token_type::left_brace:
    return "LEFT_BRACE";
  case token_type::right_brace:
    return "RIGHT_BRACE";
  case token_type::comma:
    return "COMMA";
  case token_type::dot:
    return "DOT";
  case token_type::minus:
    return "MINUS";
  case token_type::plus:
    return "PLUS";
  case token_type::semicolon:
    return "SEMICOLON";
  case token_type::star:
    return "STAR";
  case token_type::bang_equal:
    return "BANG_EQUAL";
  case token_type::bang:
    return "BANG";
  case token_type::equal_equal:
    return "EQUAL_EQUAL";
  case token_type::equal:
    return "EQUAL";
  case token_type::greater_equal:
    return "GREATER_EQUAL";
  case token_type::greater:
    return "GREATER";
  case token_type::less_equal:
    return "LESS_EQUAL";
  case token_type::less:
    return "LESS";
  case token_type::slash:
    return "SLASH";
  case token_type::string_token:
    return "STRING";
  case token_type::number:
    return "NUMBER";
  case token_type::identifier:
    return "IDENTIFIER";
  case token_type::var_token:
    return "VAR";
  case token_type::eof:
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

  if (command != "tokenize") {
    std::cerr << "Unknown command: " << command << "\n";
    return 1;
  }

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
    std::string lexeme_text = (token.type == token_type::eof)
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

    std::cout << type_name << " " << lexeme_text << " " << literal_text << "\n";
  }

  return ErrorReporter::had_error ? 65 : 0;
}
