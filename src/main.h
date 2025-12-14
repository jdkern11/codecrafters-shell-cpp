#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;
void EchoCommand(std::string arg);
void TypeCommand(std::string command, std::unordered_set<std::string> valid_commands);
void ChangeDirectoryCommand(std::string path);
std::string CleanArg(std::string arg);
std::string GetCommandPath(std::string command);
std::string GetCommandArguments(std::string command);
std::string GetCommand(std::string command);
bool IsExecutable(fs::perms);

#endif
