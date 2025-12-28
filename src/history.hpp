#ifndef SRC_HISTORY_H_
#define SRC_HISTORY_H_

#include <string>
#include <vector>

namespace shell::history {

struct Node {
  const std::string txt;
  Node* prior;
  Node* next;

  explicit Node(const std::string& txt);
};

class History {
 public:
  size_t size;
  size_t max_size;
  void insert(const std::string& txt);
  std::vector<std::string> get();
  History();
  History(size_t max_size);
  ~History();

 private:
  Node* head;
  Node* tail;
  void deleteTail();
  void deleteNode(Node* node);
};

extern History* GLOBAL_HISTORY;
std::vector<std::string> GetHistory();
}  // namespace shell::history

#endif  // SRC_HISTORY_H_
