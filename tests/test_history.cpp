#include <spdlog/spdlog.h>

#include <catch2/catch.hpp>
#include <string>
#include <vector>

#include "history.hpp"

namespace shist = shell::history;

TEST_CASE("HistoryTests", "[History]") {
  SECTION("Inserts") {
    shist::History hist{};
    REQUIRE(hist.size == 0);
    hist.insert("Hello there");
    REQUIRE(hist.size == 1);
    hist.insert("Hi there");
    REQUIRE(hist.size == 2);
    hist.insert("Ho there");
    REQUIRE(hist.size == 3);
    std::vector<std::string> expected = {"Ho there", "Hi there", "Hello there"};
    REQUIRE(hist.get() == expected);
  }

  SECTION("Deletions") {
    shist::History hist{3};
    REQUIRE(hist.size == 0);
    hist.insert("Hello there");
    REQUIRE(hist.size == 1);
    hist.insert("Hi there");
    REQUIRE(hist.size == 2);
    hist.insert("Ho there");
    REQUIRE(hist.size == 3);
    std::vector<std::string> expected = {"Ho there", "Hi there", "Hello there"};
    REQUIRE(hist.get() == expected);
    hist.insert("What's up");
    expected = {"What's up", "Ho there", "Hi there"};
    REQUIRE(hist.get() == expected);
  }

  SECTION("Single history") {
    shist::History hist{1};
    REQUIRE(hist.size == 0);
    hist.insert("Hello there");
    std::vector<std::string> expected = {"Hello there"};
    REQUIRE(hist.get() == expected);
    REQUIRE(hist.size == 1);
    hist.insert("Hi there");
    expected = {"Hi there"};
    REQUIRE(hist.get() == expected);
    REQUIRE(hist.size == 1);
    hist.insert("Ho there");
    expected = {"Ho there"};
    REQUIRE(hist.get() == expected);
    REQUIRE(hist.size == 1);
  }
}
