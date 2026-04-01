#include "evaluator/native.hpp"

#include <chrono>

#include "evaluator/runtime_error.hpp"

LoxValue get_clock(std::vector<LoxValue> &args) {
  auto now = std::chrono::system_clock::now();
  auto epoch = now.time_since_epoch();
  return static_cast<double>(
      std::chrono::duration_cast<std::chrono::seconds>(epoch).count());
}

NativeFnRegistry::NativeFnRegistry() { native_fns["clock"] = get_clock; }

LoxValue NativeFnRegistry::execute(const Token &name_token,
                                   std::vector<LoxValue> &args,
                                   const SourceContext &ctx) {
  auto name = std::string(name_token.get_lexeme(ctx));
  auto it = native_fns.find(name);
  if (it == native_fns.end()) {
    throw RuntimeError(name_token, "unknown function name");
  }

  auto fn = it->second;
  return fn(args);
}