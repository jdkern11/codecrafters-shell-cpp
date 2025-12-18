#ifndef SRC_TRIE_H_
#define SRC_TRIE_H_

#include <string>
#include <unordered_map>

struct TrieNode {
  const char letter;
  std::unordered_map<char, TrieNode*> children;
  bool isEndOfWord;

  explicit TrieNode(char letter);
};

class Trie {
 private:
  TrieNode* root;
  void deleteTrieNode(TrieNode* node);

 public:
  Trie();
  ~Trie();
  void insert(const std::string& word);
  bool contains(const std::string& word);
  int size();
};

void UpdateTrieRoot(TrieNode& root, const std::string& word);

#endif  // SRC_TRIE_H_
