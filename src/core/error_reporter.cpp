#include "error_reporter.hpp"

#include <algorithm>
#include <iostream>

#include "core/source_manager.hpp"
#include "core/span.hpp"

bool ErrorReporter::had_error = false;

// ANSI escape codes for fancy terminal styling
namespace {
const std::string ansi_reset = "\033[0m";
const std::string ansi_red = "\033[31m";
const std::string ansi_cyan = "\033[36m";
const std::string ansi_bold = "\033[1m";
} // namespace

void ErrorReporter::report(const Span &span, const std::string &message,
                           int line, const SourceContext &ctx) {
  had_error = true;
  const std::string &source = ctx.source_code;

  // 1. Find the start of the line
  int line_start = 0;
  size_t last_newline = source.rfind('\n', span.start > 0 ? span.start - 1 : 0);
  if (last_newline != std::string::npos) {
    line_start = last_newline + 1;
  }

  // 2. Find the end of the line
  int line_end = source.length();
  size_t next_newline = source.find('\n', span.start);
  if (next_newline != std::string::npos) {
    line_end = next_newline;
  }

  // 3. Extract the line text
  std::string line_text = source.substr(line_start, line_end - line_start);

  // Replace tabs with spaces so our pointer alignment math stays perfect
  std::replace(line_text.begin(), line_text.end(), '\t', ' ');

  // 4. Calculate exactly where to place the `^~~~` pointer
  int pointer_offset = span.start - line_start;
  int pointer_length = std::max(1, span.end - span.start);

  std::string pointers(pointer_offset, ' '); // Pad with spaces
  pointers += '^';                           // Point to the first char
  if (pointer_length > 1) {
    pointers.append(pointer_length - 1, '~'); // Underline the rest
  }

  // 5. Print the beautifully formatted error to stderr
  std::cerr << "[" << "line " << line << "] "
            << "Error: " << message << "\n";
}

void ErrorReporter::report_general(int line, const std::string &message) {
  had_error = true;
  std::cerr << ansi_bold << ansi_red << "[line " << line
            << "] Error: " << ansi_reset << message << "\n";
}