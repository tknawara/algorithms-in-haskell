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
  Expr ast{Literal(std::monostate{})};

  explicit Pipeline(SourceContext &context);

  bool lex();
  bool parse();
  std::pair<LoxValue, bool> eval();
};

using CommandFunc = std::function<int(Pipeline &, SourceContext &)>;

int cmd_tokenize(Pipeline &p, SourceContext &ctx);
int cmd_parse(Pipeline &p, SourceContext &ctx);
int cmd_evaluate(Pipeline &p, SourceContext &ctx);

struct CommandRegistry {
  std::unordered_map<std::string, CommandFunc> commands;

  CommandRegistry();
  int execute(const std::string &name, Pipeline &p, SourceContext &ctx);
  static void get_requirements(const std::string &name, bool &need_lex,
                               bool &need_parse, bool &need_eval);
};
