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
    auto redirection_info = ParseRedirection(user_input);
    std::ofstream write_file;
    if (redirection_info.type != RedirectType::NONE) {
      fs::path file_path{redirection_info.file};
      write_file.open(file_path, redirection_info.open_mode);
    }
    auto input = redirection_info.input;
    auto command = GetCommand(input);
    if (builtin_commands.count(command)) {
      auto args = GetCommandArguments(input);
      try {
        auto result = builtin_commands[command](args);
        if (!result.empty()) {
          if (redirection_info.type == RedirectType::OUTPUT) {
            write_file << result;
          } else {
            std::cout << result;
          }
        }
      } catch (const std::exception &e) {
        if (redirection_info.type == RedirectType::ERROR) {
          write_file << e.what();
        } else {
          std::cerr << e.what();
        }
      }
    } else {
      auto filepath = GetCommandPath(command);
      if (filepath.empty()) {
        if (redirection_info.type == RedirectType::ERROR) {
          write_file << input << ": command not found\n";
        } else {
          std::cerr << input << ": command not found\n";
        }
      } else {
        int stdoutPipe[2];
        int stderrPipe[2];
        pipe(stdoutPipe);
        pipe(stderrPipe);
        pid_t pid = fork();
        if (pid == 0) {
          dup2(stdoutPipe[1], STDOUT_FILENO);
          close(stdoutPipe[0]);
          close(stdoutPipe[1]);
          dup2(stderrPipe[1], STDERR_FILENO);
          close(stderrPipe[0]);
          close(stderrPipe[1]);
          execl("/bin/sh", "sh", "-c", input.c_str(), NULL);
          perror("execl");
          exit(1);
        } else {
          close(stdoutPipe[1]);
          close(stderrPipe[1]);

          char buffer[128];
          ssize_t bytes;
          while ((bytes = read(stdoutPipe[0], buffer, 128)) > 0) {
            if (redirection_info.type == RedirectType::OUTPUT) {
              write_file << std::string(buffer, bytes);
            } else {
              std::cout << std::string(buffer, bytes);
            }
          }
          while ((bytes = read(stderrPipe[0], buffer, 128)) > 0) {
            if (redirection_info.type == RedirectType::ERROR) {
              write_file << std::string(buffer, bytes);
            } else {
              std::cerr << std::string(buffer, bytes);
            }
          }
          waitpid(pid, NULL, 0);
        }
      }
    }
    if (write_file.is_open()) {
      write_file.close();
    }
  }
}

#endif  // SRC_MAIN_CPP_
