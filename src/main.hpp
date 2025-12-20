#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#include <functional>
#include <string>
#include <unordered_map>

void ExecuteInput(
    const std::string& user_input, int in_fd, int out_fd,
    std::unordered_map<std::string,
                       std::function<std::string(const std::string&)>>
        builtin_commands);

#endif  // SRC_MAIN_HPP_
