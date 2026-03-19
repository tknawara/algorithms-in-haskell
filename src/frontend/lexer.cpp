#include "frontend/lexer.hpp"

#include <regex>

#include "core/error_reporter.hpp"
#include "core/source_manager.hpp"

Lexer::Lexer(const SourceContext &context) : ctx(context) {}

std::vector<Token> Lexer::scan_tokens() {
  std::vector<Token> tokens;
  int current_index = 0;
  int current_line = 1;
  const std::string &source = ctx.source_code;

  static const std::vector<std::pair<std::regex, token_type>> token_regexes = {
      {std::regex(R"(\()"), token_type::left_paren},
      {std::regex(R"(\))"), token_type::right_paren},
      {std::regex(R"(\{)"), token_type::left_brace},
      {std::regex(R"(\})"), token_type::right_brace},
      {std::regex(R"(,)"), token_type::comma},
      {std::regex(R"(\.)"), token_type::dot},
      {std::regex(R"(-)"), token_type::minus},
      {std::regex(R"(\+)"), token_type::plus},
      {std::regex(R"(;)"), token_type::semicolon},
      {std::regex(R"(\*)"), token_type::star},
      {std::regex(R"(!=)"), token_type::bang_equal},
      {std::regex(R"(!)"), token_type::bang},
      {std::regex(R"(==)"), token_type::equal_equal},
      {std::regex(R"(=)"), token_type::equal},
      {std::regex(R"(>=)"), token_type::greater_equal},
      {std::regex(R"(>)"), token_type::greater},
      {std::regex(R"(<=)"), token_type::less_equal},
      {std::regex(R"(<)"), token_type::less},
      {std::regex(R"(/)"), token_type::slash},
      {std::regex(R"("[^"]*(?:"|$))"), token_type::string_token},
      {std::regex(R"(\d+(\.\d+)?)"), token_type::number},
      {std::regex(R"(and\b)"), token_type::and_token},
      {std::regex(R"(class\b)"), token_type::class_token},
      {std::regex(R"(else\b)"), token_type::else_token},
      {std::regex(R"(false\b)"), token_type::false_token},
      {std::regex(R"(fun\b)"), token_type::fun},
      {std::regex(R"(for\b)"), token_type::for_token},
      {std::regex(R"(if\b)"), token_type::if_token},
      {std::regex(R"(nil\b)"), token_type::nil_token},
      {std::regex(R"(or\b)"), token_type::or_token},
      {std::regex(R"(print\b)"), token_type::print_token},
      {std::regex(R"(return\b)"), token_type::return_token},
      {std::regex(R"(super\b)"), token_type::super_token},
      {std::regex(R"(this\b)"), token_type::this_token},
      {std::regex(R"(true\b)"), token_type::true_token},
      {std::regex(R"(var\b)"), token_type::var_token},
      {std::regex(R"(while\b)"), token_type::while_token},
      {std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"), token_type::identifier}};

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
    token_type matched_type;
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

      if (matched_type == token_type::string_token) {
        current_line +=
            std::count(lexeme_text.begin(), lexeme_text.end(), '\n');
      }

      literal_type literal_val = std::monostate{};
      bool is_valid = true;

      if (matched_type == token_type::string_token) {
        if (lexeme_text.back() != '"') {
          ErrorReporter::report(token_span, "Unterminated string.",
                                current_line, ctx);
          is_valid = false;
        } else {
          literal_val =
              std::string(lexeme_text.substr(1, lexeme_text.length() - 2));
        }
      } else if (matched_type == token_type::number) {
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

  tokens.emplace_back(token_type::eof,
                      Span(ctx.id, source.length(), source.length()),
                      std::monostate{}, current_line);
  return tokens;
}