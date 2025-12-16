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
    auto [input, output_file, error_file] = RedirectOutput("echo test 1");
    REQUIRE(input == "echo test 1");
    REQUIRE(output_file.empty());
    REQUIRE(error_file.empty());
  }

  SECTION("With >") {
    auto [input, output_file, error_file] = RedirectOutput("echo test > file.txt");
    REQUIRE(input == "echo test");
    REQUIRE(output_file == "file.txt");
    REQUIRE(error_file.empty());
  }

  SECTION("With 1>") {
    auto [input, output_file, error_file] = RedirectOutput("echo test 1> file.txt");
    REQUIRE(input == "echo test");
    REQUIRE(output_file == "file.txt");
    REQUIRE(error_file.empty());
  }

  SECTION("With 2>") {
    auto [input, output_file, error_file] = RedirectOutput("echo test 2> file.txt");
    REQUIRE(input == "echo test");
    REQUIRE(error_file == "file.txt");
    REQUIRE(output_file.empty());
  }

  SECTION("Command has number") {
    auto [input, output_file, error_file] = RedirectOutput("echo test 1> file1.txt");
    REQUIRE(input == "echo test");
    REQUIRE(output_file == "file1.txt");
    REQUIRE(error_file.empty());
  }
}

TEST_CASE("GetCommand", "[string]") {
  SECTION("Single Command") {
    REQUIRE(GetCommand(" cat /tmp/bee/f1") == "cat");
  }
}
