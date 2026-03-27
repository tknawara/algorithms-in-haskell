#include "core/format.hpp"

#include <string>

namespace format {

std::string literal(const LiteralType &value) {
  if (std::holds_alternative<std::monostate>(value)) {
    return "null";
  }
  if (std::holds_alternative<bool>(value)) {
    return std::get<bool>(value) ? "true" : "false";
  }
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  }
  if (std::holds_alternative<double>(value)) {
    double n = std::get<double>(value);
    if (n == static_cast<long long>(n)) {
      return std::to_string(static_cast<long long>(n)) + ".0";
    }
    std::string str = std::to_string(n);
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (str.back() == '.') {
      str += "0";
    }
    return str;
  }
  return "unknown";
}

std::string value(const LoxValue &val) {
  if (std::holds_alternative<std::monostate>(val)) {
    return "nil";
  }
  if (std::holds_alternative<bool>(val)) {
    return std::get<bool>(val) ? "true" : "false";
  }
  if (std::holds_alternative<std::string>(val)) {
    return std::get<std::string>(val);
  }
  if (std::holds_alternative<double>(val)) {
    double n = std::get<double>(val);
    std::string text = std::to_string(n);
    // Strip trailing zeros for clean output
    text.erase(text.find_last_not_of('0') + 1, std::string::npos);
    if (text.back() == '.') {
      text.pop_back();
    }
    return text;
  }
  return "unknown";
}

} // namespace format
