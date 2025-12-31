#include <spdlog/spdlog.h>

#include <catch2/catch.hpp>
#include <cstdio>
#include <filesystem>
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
    expected = {"Hello there", "Hi there", "Ho there"};
    REQUIRE(hist.getReverse() == expected);
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
    REQUIRE(hist.getReverse() == expected);
    REQUIRE(hist.size == 1);
    hist.insert("Hi there");
    expected = {"Hi there"};
    REQUIRE(hist.get() == expected);
    REQUIRE(hist.getReverse() == expected);
    REQUIRE(hist.size == 1);
    hist.insert("Ho there");
    expected = {"Ho there"};
    REQUIRE(hist.get() == expected);
    REQUIRE(hist.getReverse() == expected);
    REQUIRE(hist.size == 1);
  }

  SECTION("save/load") {
    shist::History hist1{5};
    hist1.insert("ls");
    hist1.insert("history");
    hist1.insert("pwd");
    hist1.insert("cd tests");
    hist1.insert("ls");
    hist1.save("test_save_file.txt", std::ios_base::out);
    shist::History hist2{4};
    hist2.load("test_save_file.txt");
    std::vector<std::string> expected = {"ls", "cd tests", "pwd", "history"};
    REQUIRE(hist2.get() == expected);
    hist2.save("test_save_file.txt", std::ios_base::out);
    shist::History hist3{5};
    hist3.load("test_save_file.txt");
    REQUIRE(hist3.get() == expected);
    hist3.save("test_save_file.txt", std::ios_base::app);
    shist::History hist4{10};
    hist4.load("test_save_file.txt");
    expected = {"ls", "cd tests", "pwd", "history",
                "ls", "cd tests", "pwd", "history"};
    REQUIRE(hist4.get() == expected);
    remove("test_save_file.txt");
  }
}
