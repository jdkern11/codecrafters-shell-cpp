#ifndef SRC_HISTORY_CPP_
#define SRC_HISTORY_CPP_

#include "./history.hpp"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

namespace hist = shell::history;

hist::Node::Node(const std::string& txt)
    : txt(txt), next(nullptr), prior(nullptr) {}

hist::History::History()
    : size(0), max_size(100), head(nullptr), tail(nullptr) {}
hist::History::History(size_t max_size)
    : size(0), max_size(max_size), head(nullptr), tail(nullptr) {}
hist::History::~History() { this->deleteNode(this->head); }

void hist::History::deleteNode(Node* node) {
  if (node == nullptr) return;
  this->deleteNode(node->next);
  delete node;
}

void hist::History::deleteTail() {
  Node* node = this->tail;
  if (node == nullptr) return;
  spdlog::debug("Deleting tail node with text \"{}\".", node->txt);
  this->tail = node->prior;
  if (this->tail != nullptr) {
    this->tail->next = nullptr;
  } else {
    this->head = nullptr;
  }
  delete node;
  this->size--;
}

void hist::History::insert(const std::string& txt) {
  Node* node = new hist::Node{txt};
  if (this->head != nullptr) {
    this->head->prior = node;
  }
  node->next = this->head;
  this->head = node;
  if (this->tail == nullptr) {
    this->tail = node;
  }
  this->size++;
  if (this->size > this->max_size) {
    this->deleteTail();
  }
  spdlog::debug("New head text is \"{}\".", this->head->txt);
}

std::vector<std::string> hist::History::get() {
  std::vector<std::string> res;
  Node* n = this->head;
  while (n != nullptr) {
    spdlog::debug("Reading node with text \"{}\".", n->txt);
    res.push_back(n->txt);
    n = n->next;
  }
  return res;
}

hist::History* hist::GLOBAL_HISTORY = nullptr;
std::vector<std::string> hist::GetHistory() {
  if (GLOBAL_HISTORY == nullptr) {
    throw std::runtime_error("Must configure `GLOBAL_HISTORY` variable.");
  }
  return GLOBAL_HISTORY->get();
}

#endif  // SRC_HISTORY_CPP_
