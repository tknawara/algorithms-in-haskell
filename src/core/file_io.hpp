#pragma once

#include <string>

// Read entire file contents into a string
// Exits with error message if file cannot be opened
std::string read_file(const std::string &filename);
