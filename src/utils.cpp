#ifndef SRC_UTILS_CPP_
#define SRC_UTILS_CPP_

#include "./utils.hpp"

#include <unistd.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "./trie.hpp"

#ifdef _WIN32
constexpr char PATH_DELIMITER = ';';  // Windows uses a semicolon for path

#else
constexpr char PATH_DELIMITER = ':';  // Linux/macOS use a colon.
#endif

std::vector<std::string> SplitText(const std::string &input, char delimiter, bool format) {
  std::vector<std::string> res;
  size_t prior_delimiter_ind = 0;
  bool in_double_quote = false;
  bool backslash = false;
  for (size_t i = 0; i < input.length(); i++) {
    if (input[i] == '\\' && !backslash && format) {
      backslash = true;
    } else if (input[i] == '"' && in_double_quote && !backslash && format) {
      Trim(FormatText(input.substr(prior_delimiter_ind, i - prior_delimiter_ind), false));
      in_double_quote = false;
    } else if (input[i] == '"' && !in_double_quote && !backslash && format) {
      prior_delimiter_ind = i;
      in_double_quote = true;
    } else if (input[i] == delimiter && ((!in_double_quote && !backslash) || !format)) {
      auto val =
          Trim(input.substr(prior_delimiter_ind, i - prior_delimiter_ind));
      if (!val.empty()) {
        if (format) {
          res.push_back(FormatText(val));
        } else {
          res.push_back(val);
        }
      }
      prior_delimiter_ind = i + 1;
    } else {
      backslash = false;
    }
  }
  // Handle last copy.
  auto val = Trim(input.substr(prior_delimiter_ind));
  if (!val.empty()) {
    if (format) {
      res.push_back(FormatText(val));
    } else {
      res.push_back(val);
    }
  }
  return res;
}

RedirectionInfo ParseRedirection(const std::string &input) {
  std::ios_base::openmode open_mode;
  RedirectType redirect_type = RedirectType::OUTPUT;
  size_t operator_size = 0;
  size_t operator_ind = std::string::npos;
  for (size_t i = 0; i < input.length(); i++) {
    if (input[i] == '>') {
      operator_size += 1;
      operator_ind = i;
      if (i > 0 && (input[i - 1] == '1' || input[i - 1] == '2')) {
        operator_ind = i - 1;
        operator_size += 1;
        redirect_type =
            input[i - 1] == '1' ? RedirectType::OUTPUT : RedirectType::ERROR;
      }
      if (i < input.length() - 1) {
        if (input[i + 1] == '>') {
          operator_size += 1;
          open_mode = std::ios_base::app;
        } else {
          open_mode = std::ios_base::out;
        }
      }
      break;
    }
  }
  if (operator_ind == std::string::npos) {
    // open_mode doesn't matter in this case.
    return RedirectionInfo{Trim(input), "", RedirectType::NONE,
                           std::ios_base::out};
  }
  std::string command = Trim(input.substr(0, operator_ind));
  std::string file =
      FormatText(Trim(input.substr(operator_ind + operator_size)));

  if (file.empty()) {
    throw std::runtime_error("Must specify an output file.");
  }
  return RedirectionInfo{
      command,
      file,
      redirect_type,
      open_mode,
  };
}

std::string Trim(std::string txt) {
  return StripEndingWhitespace(StripBeginningWhitespace(txt));
}

std::string StripBeginningWhitespace(std::string txt) {
  size_t first_non_whitespace_ind = txt.find_first_not_of(" ");
  if (first_non_whitespace_ind == std::string::npos) {
    return "";
  }
  return first_non_whitespace_ind == 0 ? txt
                                       : txt.substr(first_non_whitespace_ind);
}

std::string StripEndingWhitespace(std::string txt) {
  size_t last_non_whitespace_ind = txt.find_last_not_of(" ");
  if (last_non_whitespace_ind == std::string::npos) {
    return "";
  }
  return last_non_whitespace_ind == txt.length()
             ? txt
             : txt.substr(0, last_non_whitespace_ind + 1);
}

std::string EchoCommand(std::string arg) {
  if (!arg.empty()) {
    auto options = GetOptions(arg);
    std::string clean_arg;
    if (options.size() > 2) {
      throw std::runtime_error("Only support -e command for echo");
    }
    if (options[0] == "-e") {
      clean_arg = FormatText(options[1], true);
    } else {
      clean_arg = FormatText(options[0], false);
    }
    return clean_arg + '\n';
  } else {
    return "\n";
  }
}

std::vector<std::string> GetOptions(const std::string &input) {
  static std::unordered_set<char> not_options = {' ', '-'};
  std::vector<std::string> options;
  bool opt = false;
  size_t last_opt_ind = 0;
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c == '-') {
      opt = true;
    } else if (opt && not_options.find(c) == not_options.end() &&
               i < input.length() - 1 && input[i + 1] == ' ') {
      options.push_back(input.substr(i - 1, 2));
      opt = false;
      last_opt_ind = i + 1;
    }
  }
  if (last_opt_ind != input.length()) {
    options.push_back(
        Trim(input.substr(last_opt_ind, input.length() - last_opt_ind)));
  }
  return options;
}

std::string FormatText(std::string txt, bool option_e) {
  std::vector<char> txt_v = {};
  static std::unordered_map<char, char> valid_e_escapes = {
      {'"', '"'}, {'\\', '\\'}, {'$', '$'}, {'`', '`'}, {'n', '\n'},
  };
  static std::unordered_map<char, char> valid_doubleq_escapes = {
      {'"', '"'}, {'\\', '\\'}, {'$', '$'}, {'`', '`'}, {'\n', '\n'},
  };
  bool in_single_quote = false;
  bool in_double_quote = false;
  bool backslashed = false;
  for (char c : txt) {
    if (c == '\\' && !backslashed && !in_single_quote) {
      backslashed = true;
      continue;
    }
    if (c == '\'' && !in_double_quote && !backslashed) {
      in_single_quote = !in_single_quote;
    } else if (c == '"' && !in_single_quote && !backslashed) {
      in_double_quote = !in_double_quote;
    } else if (c == ' ' && txt_v.size() > 0 && txt_v.back() == ' ' &&
               !in_single_quote && !in_double_quote && !backslashed) {
      continue;
    } else if (in_double_quote && backslashed) {
      if (option_e) {
        auto it = valid_e_escapes.find(c);
        if (it != valid_e_escapes.end()) {
          txt_v.push_back(it->second);
        } else {
          txt_v.push_back(c);
        }
      } else {
        auto it = valid_doubleq_escapes.find(c);
        if (it != valid_e_escapes.end()) {
          txt_v.push_back(it->second);
        } else {
          txt_v.push_back('\\');
          txt_v.push_back(c);
        }
      }
      backslashed = false;
    } else {
      txt_v.push_back(c);
    }
  }
  return std::string(txt_v.begin(), txt_v.end());
}

std::string TypeCommand(std::string command,
                        std::unordered_set<std::string> valid_commands) {
  if (valid_commands.find(command) != valid_commands.end()) {
    return command + " is a shell builtin\n";
  } else {
    std::string path = GetCommandPath(command);
    if (!path.empty()) {
      return command + " is " + path + '\n';
    } else {
      throw std::runtime_error(command + ": not found\n");
    }
  }
}

std::string ChangeDirectoryCommand(std::string path) {
  if (path[0] == '~') {
    char *val = getenv("HOME");
    std::string home_dir = val == NULL ? std::string("") : std::string(val);
    path = home_dir + path.substr(1);
  }
  fs::path dir = fs::path(path);
  if (fs::exists(dir)) {
    if (fs::is_directory(dir)) {
      fs::current_path(dir);
    }
  } else {
    throw std::runtime_error("cd: " + path + ": No such file or directory\n");
  }
  return "";
}

std::pair<std::string, std::string> GetCommandAndArgs(
    const std::string &command) {
  spdlog::debug("Getting command and args");
  int first_non_whitespace_ind = command.find_first_not_of(" ");
  std::string q = command.substr(first_non_whitespace_ind);
  char delimiter = (q[0] == '\'') ? '\'' : (q[0] == '"') ? '"' : ' ';
  spdlog::debug("Delimiter is {}", delimiter);
  int end_ind = q.find_first_of(delimiter, 1);
  if (end_ind == std::string::npos) {
    return {q, ""};
  }
  std::string quoted_command = q.substr(0, end_ind + 1);
  std::string formatted_command = FormatText(quoted_command, false);
  std::string args = q.substr(end_ind + 1);
  return {Trim(formatted_command), Trim(args)};
}

void FillTrieWithPathExecutables(Trie *trie) {
  char *val = getenv("PATH");
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
      for (const auto &entry : fs::directory_iterator(loc)) {
        std::string filename = entry.path().filename().string();
        std::error_code file_ec;
        fs::perms p = fs::status(entry, file_ec).permissions();
        if (!file_ec && IsExecutable(p) && !trie->contains(filename)) {
          trie->insert(filename);
        }
      }
    } else if (fs::is_regular_file(s)) {
      std::string filename = loc_path.filename().string();
      fs::perms p = s.permissions();
      if (IsExecutable(p) && !trie->contains(filename)) {
        trie->insert(filename);
      }
    }
  }
}

std::string GetCommandPath(const std::string &command) {
  char *val = getenv("PATH");
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
      for (const auto &entry : fs::directory_iterator(loc)) {
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

#endif  // SRC_UTILS_CPP_
