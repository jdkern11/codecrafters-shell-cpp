#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  bool run = true;
  while (run) {
    std::cout << "$ ";
    std::string command;
    std::cin >> command;
    if (command == "exit") {
      run = false;
    } else {
      std::cerr << command << ": command not found\n";
    }
  }
}
