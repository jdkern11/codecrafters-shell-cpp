#ifndef SRC_TRIE_CPP_
#define SRC_TRIE_CPP_

#include "./trie.h"

#include <spdlog/spdlog.h>

#include <queue>
#include <string>
#include <unordered_map>

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
    auto map = curr->children;
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

#endif  // SRC_TRIE_CPP_
