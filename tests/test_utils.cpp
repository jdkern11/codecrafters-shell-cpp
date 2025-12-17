#include <catch2/catch.hpp>

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

TEST_CASE("ParseRedirection", "[redirect]") {
  SECTION("No >") {
    auto info = ParseRedirection("echo test 1");
    REQUIRE(info.input == "echo test 1");
    REQUIRE(info.file.empty());
    REQUIRE(info.type == RedirectType::NONE);
  }

  SECTION("With >") {
    auto info = ParseRedirection("echo test > file.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file.txt");
    REQUIRE(info.type == RedirectType::OUTPUT);
    REQUIRE(info.open_mode == std::ios_base::out);
  }

  SECTION("With >>") {
    auto info = ParseRedirection("echo test >> file.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file.txt");
    REQUIRE(info.type == RedirectType::OUTPUT);
    REQUIRE(info.open_mode == std::ios_base::app);
  }

  SECTION("With 2>>") {
    auto info = ParseRedirection("echo test 2>> file.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file.txt");
    REQUIRE(info.type == RedirectType::ERROR);
    REQUIRE(info.open_mode == std::ios_base::app);
  }

  SECTION("With 1>") {
    auto info = ParseRedirection("echo test 1> file.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file.txt");
    REQUIRE(info.type == RedirectType::OUTPUT);
    REQUIRE(info.open_mode == std::ios_base::out);
  }

  SECTION("With 2>") {
    auto info = ParseRedirection("echo test 2> file.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file.txt");
    REQUIRE(info.type == RedirectType::ERROR);
    REQUIRE(info.open_mode == std::ios_base::out);
  }

  SECTION("Command has number") {
    auto info = ParseRedirection("echo test 1> file1.txt");
    REQUIRE(info.input == "echo test");
    REQUIRE(info.file == "file1.txt");
    REQUIRE(info.type == RedirectType::OUTPUT);
    REQUIRE(info.open_mode == std::ios_base::out);
  }
}

TEST_CASE("GetCommand", "[string]") {
  SECTION("Single Command") {
    REQUIRE(GetCommand(" cat /tmp/bee/f1") == "cat");
  }
}
