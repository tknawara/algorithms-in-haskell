#pragma once

#include <string>

struct Span;
class SourceContext;

class ErrorReporter {
public:
  static bool had_error;
  static void report(const Span &span, const std::string &message, int line,
                     const SourceContext &ctx);
  static void report_general(int line, const std::string &message);
};