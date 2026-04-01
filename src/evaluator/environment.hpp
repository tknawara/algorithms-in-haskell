#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "evaluator/native.hpp"
#include "frontend/lexer.hpp"

using LoxValue = LiteralType;

// Runtime error for undefined variables
class EnvironmentError : public std::runtime_error {
public:
  explicit EnvironmentError(const std::string &message)
      : std::runtime_error(message) {}
};

// Environment stores variable bindings
// Supports nested scopes via parent pointer
class Environment {
private:
  std::unordered_map<std::string, LoxValue> values;
  Environment *parent; // nullptr for global scope

public:
  // Create global scope (no parent)
  Environment() : parent(nullptr) {}

  static NativeFnRegistry native_registry;

  // Create nested scope with parent
  explicit Environment(Environment *parent_env) : parent(parent_env) {}

  // Define a new variable in the current scope
  void define(const std::string &name, const LoxValue &value) {
    values[name] = value;
  }

  // Get a variable's value - walks up the parent chain
  LoxValue get(const std::string &name) const {
    auto it = values.find(name);
    if (it != values.end()) {
      return it->second;
    }
    if (parent != nullptr) {
      return parent->get(name);
    }
    throw EnvironmentError("Undefined variable '" + name + "'.");
  }

  // Check if a variable exists in this scope or any parent scope
  bool contains(const std::string &name) const {
    if (values.find(name) != values.end()) {
      return true;
    }
    if (parent != nullptr) {
      return parent->contains(name);
    }
    return false;
  }

  // Assign to an existing variable - walks up the parent chain
  void assign(const std::string &name, const LoxValue &value) {
    auto it = values.find(name);
    if (it != values.end()) {
      it->second = value;
      return;
    }
    if (parent != nullptr) {
      parent->assign(name, value);
      return;
    }
    throw EnvironmentError("Undefined variable '" + name + "'.");
  }
};
