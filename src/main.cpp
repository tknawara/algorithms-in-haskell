#include <fstream>
#include <iostream>
#include <sstream>

#include "core/error_reporter.hpp"
#include "evaluator/evaluator.hpp"
#include "frontend/ast.hpp"
#include "frontend/ast_printer.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"

std::string read_file(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error reading file: " << filename << "\n";
    exit(1);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./your_program.sh <command> <filename>\n";
    return 1;
  }

  std::string command = argv[1];
  std::string filename = argv[2];

  // --- INITIALIZATION ---
  std::string file_contents = read_file(filename);
  SourceContext ctx(0, filename, file_contents);

  // --- PHASE 1: LEXICAL ANALYSIS ---
  Lexer lexer(ctx);
  std::vector<Token> tokens = lexer.scan_tokens();

  if (command == "tokenize") {
    for (const auto &token : tokens) {
      std::string type_name = format_token_type(token.type);
      std::string lexeme_text = (token.type == TokenType::eof)
                                    ? ""
                                    : std::string(ctx.get_lexeme(token.span));

      // Literal formatting logic (you could extract this to a helper function
      // too!)
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
    return ErrorReporter::had_error ? 65 : 0;
  }

  // If the lexer failed, do not proceed to parsing
  if (ErrorReporter::had_error)
    return 65;

  // --- PHASE 2: PARSING ---
  Parser parser(tokens, ctx);
  Expr expression = parser.parse();

  if (command == "parse") {
    if (ErrorReporter::had_error)
      return 65;
    std::cout << ast_printer::print(expression, ctx) << "\n";
    return 0;
  }

  // If the parser failed, do not proceed to evaluation
  if (ErrorReporter::had_error)
    return 65;

  // --- PHASE 3: EVALUATION ---
  if (command == "evaluate") {
    try {
      auto result = evaluator::evaluate(expression);
      std::cout << evaluator::to_string(result) << "\n";
      return 0;
    } catch (const RuntimeError &error) {
      ErrorReporter::report_general(error.token.line, error.what());
      return 70;
    }
  }

  // --- FALLBACK ---
  std::cerr << "Unknown command: " << command << "\n";
  return 1;
}