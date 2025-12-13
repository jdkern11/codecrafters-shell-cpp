#include <iostream>
#include <string>
#include <unordered_set>

void echo_command(std::string command) {
  if (command.length() > 5) {
    std::cout << command.substr(5) << '\n';
  } else {
    std::cout << '\n';
  }
}

void type_command(
    std::string command,
    std::unordered_set<std::string> valid_commands
) {
  std::string command_query = command.substr(5);
  if (valid_commands.find(command_query) != valid_commands.end()) {
    std::cout << command_query << " is a shell builtin\n";
  } else {
    std::cout << command_query << ": not found\n";
  }
}

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
      echo_command(command);
    } else if (command.starts_with("type")) {
      type_command(command, valid_commands);
    }
    else {
      std::cerr << command << ": command not found\n";
    }
  }
}

