#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <filesystem>
#include <string>
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

enum class RedirectType { NONE, OUTPUT, ERROR };
// Holds information related to Redirection, e.g., what input is to be
// redirected, what is the file to redirect to, what information is being
// redirected (none, stdout or stderr), and what file open mode to use (write or
// append).
struct RedirectionInfo {
  const std::string input;
  const std::string file;
  const RedirectType type;
  const std::ios_base::openmode open_mode;
};
RedirectionInfo ParseRedirection(const std::string& input);
std::string StripEndingWhitespace(std::string txt);
std::string Trim(std::string txt);
bool IsExecutable(fs::perms);

#endif  // SRC_UTILS_H_
