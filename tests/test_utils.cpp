#include <catch2/catch.hpp>
#include <tuple>

#include "utils.h"

TEST_CASE("StripBeginningWhitespace", "[string]") {
  SECTION("Empty string") { REQUIRE(StripBeginningWhitespace("") == ""); }

  SECTION("No whitespace") {
    REQUIRE(StripBeginningWhitespace("hello") == "hello");
  }

  SECTION("Leading whitespace") {
    REQUIRE(StripBeginningWhitespace("   hello") == "hello");
  }

  SECTION("Trailing whitespace") {
    REQUIRE(StripBeginningWhitespace("hello   ") == "hello   ");
  }

  SECTION("Only whitespace") { REQUIRE(StripBeginningWhitespace("   ") == ""); }

  SECTION("Multiple spaces before words") {
    REQUIRE(StripBeginningWhitespace("   hello world") == "hello world");
  }
}

TEST_CASE("StripEndingWhitespace", "[string]") {
  SECTION("Empty string") { REQUIRE(StripEndingWhitespace("") == ""); }

  SECTION("No whitespace") {
    REQUIRE(StripEndingWhitespace("hello") == "hello");
  }

  SECTION("Leading whitespace") {
    REQUIRE(StripEndingWhitespace("   hello") == "   hello");
  }

  SECTION("Trailing whitespace") {
    REQUIRE(StripEndingWhitespace("hello   ") == "hello");
  }

  SECTION("Only whitespace") { REQUIRE(StripEndingWhitespace("   ") == ""); }

  SECTION("Multiple spaces after words") {
    REQUIRE(StripEndingWhitespace("hello world    ") == "hello world");
  }
}

TEST_CASE("RedirectOutput", "[redirect]") {
  SECTION("No >") {
    auto [input, file] = RedirectOutput("echo test");
    REQUIRE(input == "echo test");
    REQUIRE(file.empty());
  }

  SECTION("With >") {
    auto [input, file] = RedirectOutput("echo test > file.txt");
    REQUIRE(input == "echo test");
    REQUIRE(file == "file.txt");
  }

  SECTION("With 1>") {
    auto [input, file] = RedirectOutput("echo test 1> file.txt");
    REQUIRE(input == "echo test");
    REQUIRE(file == "file.txt");
  }
}
