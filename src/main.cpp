#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include "./main.hpp"

#include <readline/readline.h>
#include <spdlog/spdlog.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "./history.hpp"
#include "./trie.hpp"
#include "./utils.hpp"

namespace fs = std::filesystem;
namespace shist = shell::history;

bool run = true;

void sigterm_handler(int signal) {
  if (signal == SIGTERM) {
    run = false;
  }
}

int main() {
  char *val = getenv("HISTFILE");
  static const std::string kHistoryFile = val == NULL ? std::string(".shell_history") : std::string(val);
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::unordered_map<std::string,
                     std::function<std::string(const std::string &)>>
      builtin_commands = {
          {"exit",
           [](const std::string &) -> std::string {
             shist::GLOBAL_HISTORY->save(kHistoryFile, std::ios_base::out);
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
          {"history",
           [](const std::string &arg) -> std::string {
             auto history = shist::GetHistory();
             int hist_size = history.size();
             if (!arg.empty()) {
               auto args = SplitText(arg, ' ');
               for (size_t i = 0; i < args.size(); i++) {
                 if (args[i] == "-r") {
                   shist::GLOBAL_HISTORY->load(args[++i]);
                   return "";
                 } else if (args[i] == "-w") {
                   shist::GLOBAL_HISTORY->save(args[++i], std::ios_base::out);
                   return "";
                 } else if (args[i] == "-a") {
                   shist::GLOBAL_HISTORY->save(args[++i], std::ios_base::app);
                   return "";
                 } else {
                   hist_size = std::stoi(args[i]);
                   if (hist_size < 0) {
                     throw std::runtime_error("invalid option.");
                   }
                 }
               }
             }
             size_t i =
                 std::max(0, (static_cast<int>(history.size()) - hist_size));
             std::string res = "";
             for (i; i < history.size(); i++) {
               res = res + "    " + std::to_string(i + 1) + "  " + history[i] +
                     '\n';
             }
             return res;
           }},
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

  shist::History hist = shist::History{};
  shist::GLOBAL_HISTORY = &hist;
  hist.load(kHistoryFile);

  rl_completion_entry_function = &AutoComplete;
  rl_bind_key('\t', rl_complete);
  rl_bind_keyseq("\\e[A", &shist::ArrowHistory);
  rl_bind_keyseq("\\e[B", &shist::ArrowHistory);
  int in_fd = STDIN_FILENO;
  // spdlog::set_level(spdlog::level::debug);
  while (run) {
    char *char_input = readline("$ ");
    std::string user_inputs{char_input};
    free(char_input);
    if (!Trim(user_inputs).empty()) {
      hist.insert(user_inputs);
    }
    auto inputs = SplitText(user_inputs, '|');
    std::vector<std::pair<pid_t, int>> pids;
    for (size_t i = 0; i < inputs.size(); i++) {
      // Need a special exception for cd.
      auto [command, args] = GetCommandAndArgs(inputs[i]);
      if (command == "cd" || command == "history") {
        ExecuteInput(inputs[i], in_fd, STDOUT_FILENO, builtin_commands);
        continue;
      }
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
          close(pipefd[1]);
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
        in_fd = pipefd[0];
        pids.push_back({pid, pipefd[0]});
      }
    }
    bool first = true;
    for (auto &[pid, pipe] : std::ranges::views::reverse(pids)) {
      if (!first) {
        kill(pid, SIGKILL);
      }
      waitpid(pid, NULL, 0);
      first = false;
      if (pipe != STDIN_FILENO) {
        close(pipe);
      }
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
    fs::path dir_path = file_path.parent_path();
    if (!dir_path.empty() && !fs::exists(dir_path)) {
      fs::create_directories(dir_path);
    }
    write_file.open(file_path, redirection_info.open_mode);
  }
  auto input = redirection_info.input;
  spdlog::debug("Input is {}.", input);
  auto [command, args] = GetCommandAndArgs(input);
  spdlog::debug("Command is {}. Args are {}.", command, args);
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
        write_file << e.what() << '\n';
      } else {
        std::cerr << e.what() << '\n';
      }
    }
  } else {
    auto filepath = GetCommandPath(command);
    spdlog::debug("File path is {}.", filepath);
    // bool is whether it has an argument after or not.
    static std::unordered_map<std::string,
                              std::unordered_map<std::string, bool>>
        command_options = {
            {"tail", {{"-f", false}}},
            {"head", {{"-n", true}}},
        };
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
        auto split_args = SplitText(args, ' ', true);
        std::vector<char *> argv = {const_cast<char *>("stdbuf"),
                                    const_cast<char *>("-o0"),
                                    const_cast<char *>(command.c_str())};
        auto opts = command_options.find(command);
        // Needed to maintain lifetime, otherwise new_arg released from mem
        // when dropped from defined scope.
        std::vector<std::string> joined_args;
        for (size_t i = 0; i < split_args.size(); i++) {
          std::string arg = split_args[i];
          if (opts == command_options.end()) {
            spdlog::debug("Adding arg {}.", split_args[i]);
            argv.push_back(const_cast<char *>(split_args[i].c_str()));
          } else {
            auto option = opts->second.find(arg);
            if (option == opts->second.end() || !option->second) {
              spdlog::debug("Adding arg {}.", split_args[i]);
              argv.push_back(const_cast<char *>(split_args[i].c_str()));
            } else {
              std::string new_arg = arg + split_args[++i];
              joined_args.push_back(new_arg);
              spdlog::debug("Adding arg {}.", new_arg);
              argv.push_back(const_cast<char *>(new_arg.c_str()));
            }
          }
        }
        argv.push_back(nullptr);
        if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) {
          perror("setvbuf failed in child");
        }
        execv("/usr/bin/stdbuf", argv.data());
        perror("execv");
        exit(1);
      } else {
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        int kBufferSize = 1;
        char buffer[kBufferSize];
        ssize_t bytes;
        while ((bytes = read(stdoutPipe[0], buffer, kBufferSize)) > 0) {
          if (redirection_info.type == RedirectType::OUTPUT) {
            write_file << std::string(buffer, bytes);
          } else {
            std::cout << std::string(buffer, bytes) << std::flush;
          }
        }
        while ((bytes = read(stderrPipe[0], buffer, kBufferSize)) > 0) {
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
