#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "core/source_context.hpp"
#include "frontend/lexer.hpp"

using LoxValue = LiteralType;

using NativeFn = std::function<LoxValue(std::vector<LoxValue> &)>;

struct NativeFnRegistry {
  std::unordered_map<std::string, NativeFn> native_fns;

  NativeFnRegistry();
  LoxValue execute(const Token &name_token, std::vector<LoxValue> &args,
                   const SourceContext &ctx);
};

LoxValue get_clock(std::vector<LoxValue> &args);
