#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <filesystem>
#include <string>
#include <unordered_set>

namespace fs = std::filesystem;
void EchoCommand(std::string arg);
void TypeCommand(std::string command,
                 std::unordered_set<std::string> valid_commands);
void ChangeDirectoryCommand(std::string path);
std::string FormatText(std::string txt);
std::string GetCommandPath(std::string command);
std::string GetCommandArguments(std::string command);
std::string GetCommand(std::string command);
bool IsExecutable(fs::perms);

#endif  // SRC_MAIN_H_
