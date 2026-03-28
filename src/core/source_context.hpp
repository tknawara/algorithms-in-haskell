#pragma once

#include "core/span.hpp"
#include <string>

class SourceContext {
public:
  int id;
  std::string path;
  std::string source_code;

  SourceContext(int i, std::string p, std::string src)
      : id(i), path(std::move(p)), source_code(std::move(src)) {}

  std::string_view get_lexeme(const Span span) const {
    return std::string_view(source_code)
        .substr(span.start, span.end - span.start);
  }
};