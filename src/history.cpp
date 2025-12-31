#ifndef SRC_HISTORY_CPP_
#define SRC_HISTORY_CPP_

#include "./history.hpp"

#include <readline/readline.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace hist = shell::history;
namespace fs = std::filesystem;

hist::Node::Node(const std::string& txt)
    : txt(txt), next(nullptr), prior(nullptr) {}

hist::History::History() : size(0), max_size(100) { this->initalize(); }
hist::History::History(size_t max_size) : size(0), max_size(max_size) {
  this->initalize();
}
void hist::History::initalize() {
  Node* node = new hist::Node{""};
  this->head = node;
  this->tail = node;
  this->current = node;
}
hist::History::~History() { this->deleteNode(this->head); }

void hist::History::deleteNode(Node* node) {
  if (node == nullptr) return;
  this->deleteNode(node->next);
  delete node;
}

void hist::History::deleteTail() {
  Node* node = this->tail;
  if (node == this->head) return;
  spdlog::debug("Deleting tail node with text \"{}\".", node->txt);
  this->tail = node->prior;
  this->tail->next = nullptr;
  delete node;
  this->size--;
}

void hist::History::insert(const std::string& txt) {
  this->current = this->head;
  Node* node = new hist::Node{txt};
  if (this->head == this->tail) {
    this->tail = node;
  }
  // Begin at head->next to ensure init node containing user text remains at the
  // front.
  Node* old_head = this->head->next;
  if (old_head != nullptr) {
    old_head->prior = node;
  }
  node->next = old_head;
  node->prior = this->head;
  this->head->next = node;
  this->size++;
  if (this->size > this->max_size) {
    this->deleteTail();
  }
  spdlog::debug("New head text is \"{}\".", this->head->next->txt);
}

std::vector<std::string> hist::History::get() {
  std::vector<std::string> res;
  // Skip past inital node that represents current user input.
  Node* n = this->head->next;
  while (n != nullptr) {
    spdlog::debug("Reading node with text \"{}\".", n->txt);
    res.push_back(n->txt);
    n = n->next;
  }
  return res;
}

std::vector<std::string> hist::History::getReverse() {
  std::vector<std::string> res;
  Node* n = this->tail;
  while (n != this->head) {
    spdlog::debug("Reading node with text \"{}\".", n->txt);
    res.push_back(n->txt);
    n = n->prior;
  }
  return res;
}

hist::History* hist::GLOBAL_HISTORY = nullptr;
std::vector<std::string> hist::GetHistory() {
  if (GLOBAL_HISTORY == nullptr) {
    throw std::runtime_error("Must configure `GLOBAL_HISTORY` variable.");
  }
  return GLOBAL_HISTORY->getReverse();
}

void hist::History::incrementCurrent() {
  if (this->current->next != nullptr) {
    this->current = this->current->next;
  }
}
void hist::History::decrementCurrent() {
  if (this->current->prior != nullptr) {
    this->current = this->current->prior;
  }
}

std::string hist::History::getCurrentTxt() { return this->current->txt; }

void hist::History::setCurrentTxt(const std::string& txt) {
  this->current->txt = txt;
}

void hist::History::save(const std::string& filename,
                         std::ios_base::openmode open_mode) {
  std::ofstream write_file;
  fs::path file_path{filename};
  write_file.open(file_path, open_mode);
  Node* curr = this->tail;
  while (curr != this->head) {
    write_file << curr->txt;
    curr = curr->prior;
    write_file << '\n';
  }
  write_file.close();
}

void hist::History::load(const std::string& filename) {
  std::ifstream read_file;
  fs::path file_path{filename};
  read_file.open(file_path);
  if (!read_file) return;
  std::string line;
  while (std::getline(read_file, line)) {
    if (!line.empty()) {
      this->insert(line);
    }
  }
  read_file.close();
}

int hist::ArrowHistory(int count, int key) {
  char* og_text = rl_copy_text(0, rl_end);
  std::string text_copy{og_text};
  free(og_text);
  hist::GLOBAL_HISTORY->setCurrentTxt(text_copy);
  // up is 65, down is 66.
  if (key == 65) {
    hist::GLOBAL_HISTORY->incrementCurrent();
  } else if (key == 66) {
    hist::GLOBAL_HISTORY->decrementCurrent();
  }
  std::string ctxt = hist::GLOBAL_HISTORY->getCurrentTxt();
  rl_replace_line(reinterpret_cast<const char*>(ctxt.c_str()), 0);
  rl_redisplay();
  rl_point = rl_end;
  return 0;
}

#endif  // SRC_HISTORY_CPP_
