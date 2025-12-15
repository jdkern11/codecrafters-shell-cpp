#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include "./utils.h"

#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>
#include <stdexcept>
#include <tuple>

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  std::unordered_map<std::string, std::function<std::string(const std::string &)>>
      builtin_commands = {
          {"exit", [&run](const std::string &) -> std::string { 
            run = false; 
            return "";
          }},
          {"echo", EchoCommand},
          {"pwd",
           [](const std::string &) -> std::string {
             fs::path current_dir = fs::current_path();
             return current_dir.string() + '\n';
           }},
          {"cd", ChangeDirectoryCommand},
      };
  std::unordered_set<std::string> valid_commands;
  for (const auto &pair : builtin_commands) {
    valid_commands.insert(pair.first);
  }
  valid_commands.insert("type");
  builtin_commands["type"] = [&valid_commands](const std::string &input) -> std::string {
    return TypeCommand(input, valid_commands);
  };

  while (run) {
    std::cout << "$ ";
    std::string user_input;
    std::getline(std::cin, user_input);
    auto command = GetCommand(user_input);
    if (builtin_commands.count(command)) {
      auto args = GetCommandArguments(user_input);
      try {
        auto result = builtin_commands[command](args);
        if (!result.empty()) {
          std::cout << result;
        };
      } catch (const std::exception& e) {
        std::cerr << e.what();
      }
    } else {
      auto filepath = GetCommandPath(command);
      if (filepath.empty()) {
        std::cerr << user_input << ": command not found\n";
      } else {
        FILE* pipe = popen(user_input.c_str(), "r");
        if (!pipe) {
          continue;
        }
        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
          std::cout << buffer;
        }
        pclose(pipe);
      }
    }
  }
}

#endif  // SRC_MAIN_CPP_
