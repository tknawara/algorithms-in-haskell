#pragma once

#include "frontend/lexer.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>

using LoxValue = LiteralType;

// Runtime error for undefined variables
class EnvironmentError : public std::runtime_error {
public:
  explicit EnvironmentError(const std::string &message)
      : std::runtime_error(message) {}
};

// Environment stores variable bindings
// For now: single global scope only
// Later: parent pointers for nested scopes
class Environment {
private:
  std::unordered_map<std::string, LoxValue> values;

public:
  Environment() = default;

  // Define a new variable (or redefine an existing one)
  void define(const std::string &name, const LoxValue &value) {
    values[name] = value;
  }

  // Get a variable's value
  LoxValue get(const std::string &name) const {
    auto it = values.find(name);
    if (it != values.end()) {
      return it->second;
    }
    throw EnvironmentError("Undefined variable '" + name + "'.");
  }

  // Check if a variable exists
  bool contains(const std::string &name) const {
    return values.find(name) != values.end();
  }

  // Assign to an existing variable
  void assign(const std::string &name, const LoxValue &value) {
    auto it = values.find(name);
    if (it != values.end()) {
      it->second = value;
      return;
    }
    throw EnvironmentError("Undefined variable '" + name + "'.");
  }
};
