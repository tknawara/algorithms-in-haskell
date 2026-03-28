#include <iostream>
#include <string>

#include "commands.hpp"
#include "core/file_io.hpp"
#include "core/source_manager.hpp"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./your_program.sh <command> <filename>\n";
    return 1;
  }

  std::string command = argv[1];
  std::string filename = argv[2];

  // Determine what phases we need
  bool need_lex = false, need_parse = false, need_eval = false;
  CommandRegistry::get_requirements(command, need_lex, need_parse, need_eval);

  if (!need_lex) {
    std::cerr << "Unknown command: " << command << "\n";
    return 1;
  }

  // Setup
  std::string file_contents = read_file(filename);
  SourceContext ctx(0, filename, file_contents);
  Pipeline pipeline(ctx);

  // Execute required phases (continue even on errors, command will handle exit
  // code)
  pipeline.lex();

  if (need_parse) {
    // Use the appropriate parsing method based on the command
    if (command == "evaluate") {
      pipeline.parse_expression();
    } else if (command == "run") {
      pipeline.parse_program();
    } else {
      // For parse command, we still use expression parsing for backward compatibility
      pipeline.parse_expression();
    }
  }

  // Dispatch to command handler
  CommandRegistry registry;
  return registry.execute(command, pipeline, ctx);
}
