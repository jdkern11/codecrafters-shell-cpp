#include "main.h"
#include <iostream>
#include <unistd.h>
#include <unordered_map>
#include <functional>

#ifdef _WIN32
    constexpr char PATH_DELIMITER = ';'; // Windows uses a semicolon for path separation in environment variables.
#else
    constexpr char PATH_DELIMITER = ':'; // Linux/macOS use a colon.
#endif


int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  std::unordered_map<std::string, std::function<void(const std::string&)>> builtin_commands = {
    {"exit", [&run](const std::string&) { run = false; }},
    {"echo", EchoCommand},
    {"pwd", [](const std::string&) { 
      fs::path current_dir = fs::current_path();
      std::cout << current_dir.string() << '\n';
    }},
    {"cd", ChangeDirectoryCommand},
  };
  std::unordered_set<std::string> valid_commands;
  for (const auto& pair : builtin_commands) {
    valid_commands.insert(pair.first);
  }
  valid_commands.insert("type");
  builtin_commands["type"] = [&valid_commands](const std::string& input) { 
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
      } else  {
        system(user_input.c_str());
      }
    }
  }
}

void EchoCommand(std::string arg) {
  if (!arg.empty()) {
    std::cout << arg << '\n';
  } else {
    std::cout << '\n';
  }
}

void TypeCommand(
    std::string command,
    std::unordered_set<std::string> valid_commands
) {
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
    fs::path dir = fs::path(path);
    if (fs::exists(dir)) {
      if (fs::is_directory(dir)) {
        fs::current_path(dir);
      }
    } else {
      std::cout << "cd: " << path << ": No such file or directory";
    }
}

std::string GetCommand(std::string command) {
  int first_non_whitespace_ind = command.find_first_not_of(" ");
  std::string command_query = command.substr(first_non_whitespace_ind);
  int end_ind = command_query.find_first_of(" ");
  return command_query.substr(0, end_ind);
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
  char * val = getenv("PATH");
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
      for (const auto& entry: fs::directory_iterator(loc)) {
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
