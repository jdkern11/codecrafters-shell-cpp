#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <filesystem>
#include <string>
#include <tuple>
#include <unordered_set>

namespace fs = std::filesystem;
std::string EchoCommand(std::string arg);
std::string TypeCommand(std::string command,
                        std::unordered_set<std::string> valid_commands);
std::string ChangeDirectoryCommand(std::string path);
std::string FormatText(std::string txt);
std::string GetCommandPath(std::string command);
std::string GetCommandArguments(std::string command);
std::string GetCommand(std::string command);
std::string StripBeginningWhitespace(std::string txt);
std::tuple<std::string, std::string> RedirectOutput(std::string input);
std::string StripEndingWhitespace(std::string txt);
std::string Trim(std::string txt);
bool IsExecutable(fs::perms);

#endif  // SRC_UTILS_H_
