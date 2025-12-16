#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "./utils.h"

namespace fs = std::filesystem;

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
    auto [input, output_file, error_file] = RedirectOutput(user_input);
    auto command = GetCommand(input);
    if (builtin_commands.count(command)) {
      auto args = GetCommandArguments(input);
      try {
        auto result = builtin_commands[command](args);
        if (!result.empty()) {
          if (output_file.empty()) {
            std::cout << result;
          } else {
            fs::path filePath{output_file};
            std::ofstream outFile{filePath};
            if (outFile.is_open()) {
              outFile << result;
              outFile.close();
            }
          }
        }
      } catch (const std::exception &e) {
        if (error_file.empty()) {
          std::cerr << e.what();
        } else {
          fs::path filePath{error_file};
          std::ofstream outFile{filePath};
          if (outFile.is_open()) {
            outFile << e.what();
            outFile.close();
          }
        }
      }
    } else {
      auto filepath = GetCommandPath(command);
      if (filepath.empty()) {
        if (error_file.empty()) {
          std::cerr << input << ": command not found\n";
        } else {
          fs::path filePath{error_file};
          std::ofstream outFile{filePath};
          if (outFile.is_open()) {
            outFile << input << ": command not found\n";
            outFile.close();
          }
        }
      } else {
        int stdoutPipe[2];
        int stderrPipe[2];
        pipe(stdoutPipe);
        pipe(stderrPipe);
        pid_t pid = fork();
        if (pid == 0) {
          dup2(stdoutPipe[1], STDOUT_FILENO);
          dup2(stderrPipe[1], STDERR_FILENO);
          close(stdoutPipe[0]);
          close(stdoutPipe[1]);
          execl("/bin/sh", "sh", "-c", input.c_str(), NULL);
        } else {
          close(stdoutPipe[1]);
          close(stderrPipe[1]);

          char buffer[128];
          ssize_t bytes;
          if (output_file.empty()) {
            while ((bytes = read(stdoutPipe[0], buffer, 128)) > 0) {
              std::cout << std::string(buffer, bytes);
            }
          } else {
            fs::path filePath{output_file};
            std::ofstream outFile{filePath};
            if (outFile.is_open()) {
              while ((bytes = read(stdoutPipe[0], buffer, 128)) > 0) {
                outFile << std::string(buffer, bytes);
              }
              outFile.close();
            }
          }
          if (error_file.empty()) {
            while ((bytes = read(stderrPipe[0], buffer, 128)) > 0) {
              std::cerr << std::string(buffer, bytes);
            }
          } else {
            fs::path filePath{error_file};
            std::ofstream outFile{filePath};
            if (outFile.is_open()) {
              while ((bytes = read(stderrPipe[0], buffer, 128)) > 0) {
                outFile << std::string(buffer, bytes);
              }
              outFile.close();
            }
          }
          waitpid(pid, NULL, 0);
        }
      }
    }
  }
}

#endif  // SRC_MAIN_CPP_
