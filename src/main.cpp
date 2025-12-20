#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include "./main.hpp"

#include <readline/readline.h>
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

#include "./trie.hpp"
#include "./utils.hpp"

namespace fs = std::filesystem;

bool run = true;

void sigterm_handler(int signal) {
  if (signal == SIGTERM) {
    run = false;
  }
}

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::unordered_map<std::string,
                     std::function<std::string(const std::string &)>>
      builtin_commands = {
          {"exit",
           [](const std::string &) -> std::string {
             kill(getppid(), SIGTERM);
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

  Trie trie = Trie{};
  trie.insert("echo");
  trie.insert("exit");
  FillTrieWithPathExecutables(&trie);
  GLOBAL_TRIE = &trie;

  rl_completion_entry_function = &AutoComplete;
  rl_bind_key('\t', rl_complete);
  int in_fd = STDIN_FILENO;
  while (run) {
    char *char_input = readline("$ ");
    std::string user_inputs{char_input};
    free(char_input);
    auto inputs = GetPipes(user_inputs);
    int prior_input = STDIN_FILENO;
    for (size_t i = 0; i < inputs.size(); i++) {
      int pipefd[2];
      if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
      }

      pid_t pid = fork();
      if (pid == -1) {
        perror("fork");
        exit(1);
      }
      if (pid == 0) {
        close(pipefd[0]);
        if (i < inputs.size() - 1) {
          ExecuteInput(inputs[i], in_fd, pipefd[1], builtin_commands);
        } else {
          ExecuteInput(inputs[i], in_fd, STDOUT_FILENO, builtin_commands);
        }
        exit(0);
      } else {
        close(pipefd[1]);
        struct sigaction sa;
        sa.sa_handler = sigterm_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGTERM, &sa, NULL) == -1) {
          perror("sigaction");
          return 1;
        }
        if (prior_input != STDIN_FILENO) {
          close(prior_input);
        }
        prior_input = in_fd;
        in_fd = pipefd[0];
        waitpid(pid, NULL, 0);
      }
    }
    if (prior_input != STDIN_FILENO) {
      close(prior_input);
    }
    in_fd = STDIN_FILENO;
  }
}

void ExecuteInput(
    const std::string &user_input, int in_fd, int out_fd,
    std::unordered_map<std::string,
                       std::function<std::string(const std::string &)>>
        builtin_commands) {
  if (in_fd != STDIN_FILENO) {
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  }
  if (out_fd != STDOUT_FILENO) {
    dup2(out_fd, STDOUT_FILENO);
    if (out_fd != STDIN_FILENO) {
      close(out_fd);
    }
  }
  auto redirection_info = ParseRedirection(user_input);
  std::ofstream write_file;
  if (redirection_info.type != RedirectType::NONE) {
    fs::path file_path{redirection_info.file};
    write_file.open(file_path, redirection_info.open_mode);
  }
  auto input = redirection_info.input;
  auto command = GetCommand(input);
  auto args = GetCommandArguments(input);
  if (builtin_commands.count(command)) {
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
        if (args.empty()) {
          execl(filepath.c_str(), command.c_str(), NULL);
        } else {
          execl(filepath.c_str(), command.c_str(), args.c_str(), NULL);
        }
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
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        waitpid(pid, NULL, 0);
      }
    }
  }
  if (write_file.is_open()) {
    write_file.close();
  }
}

#endif  // SRC_MAIN_CPP_
