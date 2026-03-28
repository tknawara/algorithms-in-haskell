#pragma once

#include <string>

struct Span;
class SourceContext;
struct Token;

class ErrorReporter {
public:
  static bool had_error;
  static void report(const Span &span, const std::string &message, int line,
                     const SourceContext &ctx);
  static void report_general(int line, const std::string &message);
  
  // Convenience: report error at a token's location
  static void report_token(const Token &token, const std::string &message,
                           const SourceContext &ctx);
};