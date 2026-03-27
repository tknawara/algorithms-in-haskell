#include "core/file_io.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

std::string read_file(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error reading file: " << filename << "\n";
    std::exit(1);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
