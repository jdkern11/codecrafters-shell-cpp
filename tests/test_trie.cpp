#include <spdlog/spdlog.h>

#include <catch2/catch.hpp>

#include "trie.h"

TEST_CASE("UpdateTrieRoot", "[Trie]") {
  SECTION("") {
    spdlog::set_level(spdlog::level::debug);
    Trie trie = Trie{};
    REQUIRE(trie.size() == 0);
    REQUIRE(!trie.contains("hello"));
    trie.insert("hello");
    REQUIRE(trie.contains("hello"));
    REQUIRE(trie.size() == 5);
    REQUIRE(!trie.contains("hell"));
    trie.insert("hell");
    REQUIRE(trie.contains("hell"));
    REQUIRE(trie.size() == 5);
  }
}
