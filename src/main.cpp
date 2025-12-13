#include <iostream>
#include <string>

void echo_command(std::string command) {
    if (command.length() > 5) {
      std::cout << command.substr(5) << '\n';
    } else {
      std::cout << '\n';
    }
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  while (run) {
    std::cout << "$ ";
    std::string command;
    std::getline(std::cin, command);
    if (command == "exit") {
      run = false;
    } else if (command.starts_with("echo")) {
      echo_command(command);
    }
    else {
      std::cerr << command << ": command not found\n";
    }
  }
}

