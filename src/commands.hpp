#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/source_manager.hpp"
#include "evaluator/evaluator.hpp"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"

struct Pipeline {
  SourceContext &ctx;
  std::vector<Token> tokens;
  
  // For expression-only parsing (evaluate command)
  Expr expr_ast{Literal(std::monostate{})};
  
  // For statement parsing (run command)
  Program program;
  
  // Environment for variable storage (shared across program execution)
  Environment env;

  explicit Pipeline(SourceContext &context);

  bool lex();
  bool parse_expression();  // Parse a single expression (for evaluate command)
  bool parse_program();     // Parse a full program with statements (for run command)
  
  std::pair<LoxValue, bool> eval();
  bool run();               // Execute statements
};

using CommandFunc = std::function<int(Pipeline &, SourceContext &)>;

int cmd_tokenize(Pipeline &p, SourceContext &ctx);
int cmd_parse(Pipeline &p, SourceContext &ctx);
int cmd_evaluate(Pipeline &p, SourceContext &ctx);
int cmd_run(Pipeline &p, SourceContext &ctx);

struct CommandRegistry {
  std::unordered_map<std::string, CommandFunc> commands;

  CommandRegistry();
  int execute(const std::string &name, Pipeline &p, SourceContext &ctx);
  static void get_requirements(const std::string &name, bool &need_lex,
                               bool &need_parse, bool &need_eval);
};
