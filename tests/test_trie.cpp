#include <spdlog/spdlog.h>

#include <catch2/catch.hpp>

#include "trie.hpp"

TEST_CASE("UpdateTrieRoot", "[Trie]") {
  SECTION("Inserts") {
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

  SECTION("Get Words") {
    // spdlog::set_level(spdlog::level::debug);
    Trie trie = Trie{};
    trie.insert("hello");
    trie.insert("hell");
    trie.insert("help");
    trie.insert("go");
    trie.insert("goon");
    trie.insert("away");

    std::vector<std::string> ans = trie.getWords("h");
    std::sort(ans.begin(), ans.end());
    REQUIRE(ans == std::vector<std::string>{"hell", "hello", "help"});
    REQUIRE(trie.getWords("i") == std::vector<std::string>{});
    REQUIRE(trie.getWords("g") == std::vector<std::string>{"go", "goon"});
  }
}
