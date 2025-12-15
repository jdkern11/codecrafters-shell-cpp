#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "./utils.h"

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  std::unordered_map<std::string,
                     std::function<std::string(const std::string &)>>
      builtin_commands = {
          {"exit",
           [&run](const std::string &) -> std::string {
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
  builtin_commands["type"] =
      [&valid_commands](const std::string &input) -> std::string {
    return TypeCommand(input, valid_commands);
  };

  while (run) {
    std::cout << "$ ";
    std::string user_input;
    std::getline(std::cin, user_input);
    auto [input, file] = RedirectOutput(user_input);
    auto command = GetCommand(input);
    if (builtin_commands.count(command)) {
      auto args = GetCommandArguments(input);
      try {
        auto result = builtin_commands[command](args);
        if (!result.empty()) {
          if (file.empty()) {
            std::cout << result;
          } else {
            fs::path filePath(file);
            std::ofstream outFile(filePath);
            if (outFile.is_open()) {
              outFile << result;
              outFile.close();
            }
          }
        }
      } catch (const std::exception &e) {
        std::cerr << e.what();
      }
    } else {
      auto filepath = GetCommandPath(command);
      if (filepath.empty()) {
        std::cerr << user_input << ": command not found\n";
      } else {
        FILE *pipe = popen(user_input.c_str(), "r");
        if (!pipe) {
          continue;
        }
        char buffer[128];
        if (file.empty()) {
          while (fgets(buffer, 128, pipe) != NULL) {
            std::cout << buffer;
          }
        } else {
          fs::path filePath(file);
          std::ofstream outFile(filePath);
          if (outFile.is_open()) {
            while (fgets(buffer, 128, pipe) != NULL) {
              outFile << buffer;
            }
            outFile.close();
          }
        }
        pclose(pipe);
      }
    }
  }
}

#endif  // SRC_MAIN_CPP_
