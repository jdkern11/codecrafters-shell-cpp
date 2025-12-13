#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <unordered_set>
#include <filesystem>

void EchoCommand(std::string command);
void TypeCommand(std::string command, std::unordered_set<std::string> valid_commands);
std::string GetCommandPath(std::string command);
std::string GetCommandArguments(std::string command);

#endif
