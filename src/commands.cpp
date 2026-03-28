#include "commands.hpp"

#include <iostream>

#include "core/error_reporter.hpp"
#include "core/format.hpp"
#include "evaluator/environment.hpp"
#include "evaluator/evaluator.hpp"
#include "frontend/ast_printer.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"

Pipeline::Pipeline(SourceContext &context) : ctx(context) {}

bool Pipeline::lex() {
  Lexer lexer(ctx);
  tokens = lexer.scan_tokens();
  return !ErrorReporter::had_error;
}

// Parse a single expression (for evaluate command)
bool Pipeline::parse_expression() {
  Parser parser(tokens, ctx);
  expr_ast = parser.parse_expression_only();
  return !ErrorReporter::had_error;
}

// Parse a full program with statements (for run command)
bool Pipeline::parse_program() {
  Parser parser(tokens, ctx);
  program = parser.parse();
  return !ErrorReporter::had_error;
}

std::pair<LoxValue, bool> Pipeline::eval() {
  try {
    LoxValue result = evaluator::evaluate(expr_ast, env, ctx);
    return {result, true};
  } catch (const RuntimeError &error) {
    ErrorReporter::report_general(error.token.line, error.what());
    return {std::monostate{}, false};
  }
}

// Execute statements
bool Pipeline::run() {
  try {
    evaluator::execute_program(program, env, ctx);
    return true;
  } catch (const RuntimeError &error) {
    ErrorReporter::report_general(error.token.line, error.what());
    return false;
  } catch (const EnvironmentError &error) {
    ErrorReporter::report_general(0, error.what());
    return false;
  }
}

int cmd_tokenize(Pipeline &p, SourceContext &ctx) {
  for (const auto &token : p.tokens) {
    std::string type_name = format_token_type(token.type);
    std::string lexeme_text = (token.type == TokenType::eof)
                                  ? ""
                                  : std::string(token.get_lexeme(ctx));
    std::cout << type_name << " " << lexeme_text << " "
              << format::literal(token.literal) << "\n";
  }
  return ErrorReporter::had_error ? 65 : 0;
}

int cmd_parse(Pipeline &p, SourceContext &ctx) {
  if (ErrorReporter::had_error)
    return 65;
  std::cout << ast_printer::print(p.expr_ast, ctx) << "\n";
  return 0;
}

int cmd_evaluate(Pipeline &p, SourceContext & /*ctx*/) {
  if (ErrorReporter::had_error)
    return 65;
  auto [result, success] = p.eval();
  if (!success)
    return 70;
  std::cout << format::value(result) << "\n";
  return 0;
}

int cmd_run(Pipeline &p, SourceContext & /*ctx*/) {
  if (ErrorReporter::had_error)
    return 65;
  if (!p.run())
    return 70;
  return 0;
}

CommandRegistry::CommandRegistry() {
  commands["tokenize"] = cmd_tokenize;
  commands["parse"] = cmd_parse;
  commands["evaluate"] = cmd_evaluate;
  commands["run"] = cmd_run;
}

int CommandRegistry::execute(const std::string &name, Pipeline &p,
                             SourceContext &ctx) {
  auto it = commands.find(name);
  if (it == commands.end()) {
    std::cerr << "Unknown command: " << name << "\n";
    return 1;
  }
  return it->second(p, ctx);
}

void CommandRegistry::get_requirements(const std::string &name, bool &need_lex,
                                       bool &need_parse, bool &need_eval) {
  need_lex = need_parse = need_eval = false;

  if (name == "tokenize") {
    need_lex = true;
  } else if (name == "parse") {
    need_lex = need_parse = true;
  } else if (name == "evaluate") {
    need_lex = need_parse = need_eval = true;
  } else if (name == "run") {
    need_lex = need_parse = need_eval = true;
  }
}
