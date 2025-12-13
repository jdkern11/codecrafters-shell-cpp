#include "main.h"

#ifdef _WIN32
    constexpr char PATH_DELIMITER = ';'; // Windows uses a semicolon for path separation in environment variables.
#else
    constexpr char PATH_DELIMITER = ':'; // Linux/macOS use a colon.
#endif


int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::unordered_set<std::string> valid_commands;
  valid_commands.insert("exit");
  valid_commands.insert("echo");
  valid_commands.insert("type");

  bool run = true;
  while (run) {
    std::cout << "$ ";
    std::string command;
    std::getline(std::cin, command);
    if (command == "exit") {
      run = false;
    } else if (command.starts_with("echo")) {
      EchoCommand(command);
    } else if (command.starts_with("type")) {
      TypeCommand(command, valid_commands);
    }
    else {
      std::cerr << command << ": command not found\n";
    }
  }
}

void EchoCommand(std::string command) {
  if (command.length() > 5) {
    std::cout << command.substr(5) << '\n';
  } else {
    std::cout << '\n';
  }
}

void TypeCommand(
    std::string command,
    std::unordered_set<std::string> valid_commands
) {
  std::string command_query = GetCommandArguments(command);
  if (valid_commands.find(command_query) != valid_commands.end()) {
    std::cout << command_query << " is a shell builtin\n";
  } else {
    std::string command_path = GetCommandPath(command_query);
    if (command_path.empty()) {
      std::cout << command_query << ": not found\n";
    }
    else {
      std::cout << command_query << " is " << command_path << '\n';
    }
  }
}

std::string GetCommandArguments(std::string command) {
  int first_whitespace_ind = command.find_first_of(" ");
  std::string command_query = command.substr(first_whitespace_ind);
  int start_ind = command_query.find_first_not_of(" ");
  int end_ind = command_query.find_last_not_of(" ");
  return command_query.substr(start_ind, end_ind - start_ind + 1);
}

std::string GetCommandPath(std::string command) {
  char * val = getenv("PATH");
  std::string path = val == NULL ? std::string("") : std::string(val);
  std::stringstream ss(path);
  std::string folder;
  while (std::getline(ss, folder, PATH_DELIMITER)) {
    try {
      for (const auto& entry: fs::directory_iterator(folder)) {
        std::string loc = entry.path().string();
        std::string filename = entry.path().filename().string();
        if (command == filename) {
          std::error_code ec;
          fs::perms p = fs::status(entry, ec).permissions();
          if (!ec) {
            bool executable = (p & fs::perms::owner_exec) != fs::perms::none ||
                              (p & fs::perms::group_exec) != fs::perms::none ||
                              (p & fs::perms::others_exec) != fs::perms::none;
            if (executable) {
              return loc;
            }
          }
        }
      }
    } catch (const fs::filesystem_error& e) {
      std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }
  }
  return "";
}
