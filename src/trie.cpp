#ifndef SRC_TRIE_CPP_
#define SRC_TRIE_CPP_

#include "./trie.h"

#include <readline/readline.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

TrieNode::TrieNode(char letter) : letter(letter), isEndOfWord(false) {}
Trie::Trie() : root(new TrieNode('\0')) {}

Trie::~Trie() { deleteTrieNode(this->root); }

void Trie::deleteTrieNode(TrieNode* node) {
  if (node == nullptr) return;
  for (auto& child : node->children) {
    deleteTrieNode(child.second);
  }
  delete node;
}

void Trie::insert(const std::string& word) {
  TrieNode* curr = this->root;
  for (char c : word) {
    auto& map = curr->children;
    if (map.find(c) != map.end()) {
      spdlog::debug("{} already exists in node {}.", c, curr->letter);
      curr = map[c];
    } else {
      TrieNode* node = new TrieNode{c};
      spdlog::debug("Adding character {} to node {}.", c, curr->letter);
      map[c] = node;
      curr = node;
    }
  }
  curr->isEndOfWord = true;
}

bool Trie::contains(const std::string& word) {
  TrieNode* curr = this->root;
  for (char c : word) {
    auto& map = curr->children;
    if (map.find(c) != map.end()) {
      spdlog::debug("trie contains {}.", c);
      curr = map[c];
    } else {
      return false;
    }
  }
  if (curr->isEndOfWord) {
    spdlog::debug("trie contains {}.", word);
  } else {
    spdlog::debug("trie has {}, but it isn't flagged.", word);
  }
  return curr->isEndOfWord;
}

int Trie::size() {
  int count = 0;
  std::queue<TrieNode*> queue;
  queue.push(this->root);
  while (!queue.empty()) {
    TrieNode* curr = queue.front();
    queue.pop();
    count++;
    for (auto& child : curr->children) {
      queue.push(child.second);
    }
  }
  // Don't want to include count of root node.
  count--;
  return count;
}

std::vector<std::string> Trie::getWords(const std::string& prefix) {
  std::vector<std::string> words;
  TrieNode* curr = this->root;
  for (auto c : prefix) {
    if (curr->children.find(c) != curr->children.end()) {
      curr = curr->children[c];
    } else {
      return words;
    }
  }

  std::queue<std::pair<TrieNode*, std::string>> queue;
  if (curr != this->root) {
    queue.emplace(curr, prefix);
  }
  while (!queue.empty()) {
    auto [node, word] = queue.front();
    queue.pop();
    if (node->isEndOfWord) {
      words.push_back(word);
    }
    for (auto& child : node->children) {
      queue.emplace(child.second, word + child.first);
    }
  }
  return words;
}

Trie* GLOBAL_TRIE = nullptr;
char* AutoComplete(const char* text, int state) {
  static std::vector<std::string> matches;
  static size_t match_index = 0;
  if (GLOBAL_TRIE == nullptr) {
    throw std::runtime_error("Must configure `GLOBAL_TRIE` variable.");
  }

  if (state == 0) {
    matches = GLOBAL_TRIE->getWords(text);
    match_index = 0;
  }
  return (match_index < matches.size()) ? strdup(matches[match_index++].c_str())
                                        : reinterpret_cast<char*>(NULL);
}

#endif  // SRC_TRIE_CPP_
