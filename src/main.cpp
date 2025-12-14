#ifndef SRC_MAIN_CPP_
#define SRC_MAIN_CPP_

#include "./main.h"

#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
constexpr char PATH_DELIMITER = ';';  // Windows uses a semicolon for path

#else
constexpr char PATH_DELIMITER = ':';  // Linux/macOS use a colon.
#endif

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  std::unordered_map<std::string, std::function<void(const std::string &)>>
      builtin_commands = {
          {"exit", [&run](const std::string &) { run = false; }},
          {"echo", EchoCommand},
          {"pwd",
           [](const std::string &) {
             fs::path current_dir = fs::current_path();
             std::cout << current_dir.string() << '\n';
           }},
          {"cd", ChangeDirectoryCommand},
      };
  std::unordered_set<std::string> valid_commands;
  for (const auto &pair : builtin_commands) {
    valid_commands.insert(pair.first);
  }
  valid_commands.insert("type");
  builtin_commands["type"] = [&valid_commands](const std::string &input) {
    TypeCommand(input, valid_commands);
  };

  while (run) {
    std::cout << "$ ";
    std::string user_input;
    std::getline(std::cin, user_input);
    auto command = GetCommand(user_input);
    if (builtin_commands.count(command)) {
      auto args = GetCommandArguments(user_input);
      builtin_commands[command](args);
    } else {
      auto filepath = GetCommandPath(command);
      if (filepath.empty()) {
        std::cerr << user_input << ": command not found\n";
      } else {
        system(user_input.c_str());
      }
    }
  }
}

void EchoCommand(std::string arg) {
  if (!arg.empty()) {
    auto clean_arg = FormatText(arg);
    std::cout << clean_arg << '\n';
  } else {
    std::cout << '\n';
  }
}

std::string FormatText(std::string txt) {
  std::vector<char> txt_v = {};
  std::unordered_set<char> valid_doubleq_escapes = {'"', '\\', '$', '`', '\n'};
  bool in_single_quote = false;
  bool in_double_quote = false;
  bool backslashed = false;
  for (char c : txt) {
    if (c == '\\' && !backslashed && !in_single_quote) {
      backslashed = true;
      continue;
    }
    if (c == '\'' && !in_double_quote && !backslashed) {
      in_single_quote = !in_single_quote;
    } else if (c == '"' && !in_single_quote && !backslashed) {
      in_double_quote = !in_double_quote;
    } else if (c == ' ' && txt_v.size() > 0 && txt_v.back() == ' ' &&
               !in_single_quote && !in_double_quote && !backslashed) {
      continue;
    } else {
      if (in_double_quote && backslashed &&
          valid_doubleq_escapes.find(c) == valid_doubleq_escapes.end()) {
        txt_v.push_back('\\');
      }
      txt_v.push_back(c);
      backslashed = false;
    }
  }
  return std::string(txt_v.begin(), txt_v.end());
}

void TypeCommand(std::string command,
                 std::unordered_set<std::string> valid_commands) {
  if (valid_commands.find(command) != valid_commands.end()) {
    std::cout << command << " is a shell builtin\n";
  } else {
    std::string path = GetCommandPath(command);
    if (!path.empty()) {
      std::cout << command << " is " << path << '\n';
    } else {
      std::cout << command << ": not found\n";
    }
  }
}

void ChangeDirectoryCommand(std::string path) {
  if (path[0] == '~') {
    char *val = getenv("HOME");
    std::string home_dir = val == NULL ? std::string("") : std::string(val);
    path = home_dir + path.substr(1);
  }
  fs::path dir = fs::path(path);
  if (fs::exists(dir)) {
    if (fs::is_directory(dir)) {
      fs::current_path(dir);
    }
  } else {
    std::cout << "cd: " << path << ": No such file or directory\n";
  }
}

std::string GetCommand(std::string command) {
  int first_non_whitespace_ind = command.find_first_not_of(" ");
  std::string q = command.substr(first_non_whitespace_ind);
  char delimiter = (q[0] == '\'') ? '\'' : (q[0] == '"') ? '"' : ' ';
  int end_ind = q.find_first_of(delimiter, 1);
  if (delimiter == ' ') {
    return q.substr(0, end_ind);
  }
  std::string quoted_command = q.substr(0, end_ind + 1);
  std::string formatted_command = FormatText(quoted_command);
  return formatted_command;
}

std::string GetCommandArguments(std::string command) {
  int first_whitespace_ind = command.find_first_of(" ");
  if (first_whitespace_ind == std::string::npos) {
    return "";
  }
  std::string command_args = command.substr(first_whitespace_ind);
  int start_ind = command_args.find_first_not_of(" ");
  int end_ind = command_args.find_last_not_of(" ");
  return command_args.substr(start_ind, end_ind - start_ind + 1);
}

std::string GetCommandPath(std::string command) {
  char *val = getenv("PATH");
  std::string path = val == NULL ? std::string("") : std::string(val);
  std::stringstream ss(path);
  std::string loc;
  while (std::getline(ss, loc, PATH_DELIMITER)) {
    std::error_code loc_ec;
    fs::path loc_path = fs::path(loc);
    fs::file_status s = fs::status(loc_path, loc_ec);
    if (loc_ec) {
      continue;
    } else if (fs::is_directory(s)) {
      for (const auto &entry : fs::directory_iterator(loc)) {
        std::string filename = entry.path().filename().string();
        if (command == filename) {
          std::error_code file_ec;
          fs::perms p = fs::status(entry, file_ec).permissions();
          if (!file_ec && IsExecutable(p)) {
            return entry.path().string();
          }
        }
      }
    } else if (fs::is_regular_file(s)) {
      std::string filename = loc_path.filename().string();
      if (command == filename) {
        fs::perms p = s.permissions();
        if (IsExecutable(p)) {
          return loc;
        }
      }
    }
  }
  return "";
}

bool IsExecutable(fs::perms p) {
  return (p & fs::perms::owner_exec) != fs::perms::none ||
         (p & fs::perms::group_exec) != fs::perms::none ||
         (p & fs::perms::others_exec) != fs::perms::none;
}

#endif  // SRC_MAIN_CPP_
