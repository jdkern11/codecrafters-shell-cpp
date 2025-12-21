#include <catch2/catch.hpp>
#include <vector>

#include "utils.hpp"

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

TEST_CASE("GetCommandAndArgs", "[string]") {
  SECTION("Single Arg") {
    auto [command, arg] = GetCommandAndArgs(" cat /tmp/bee/f1");
    REQUIRE(command == "cat");
    REQUIRE(arg == "/tmp/bee/f1");
  }

  SECTION("No Arg") {
    auto [command, arg] = GetCommandAndArgs(" ls ");
    REQUIRE(command == "ls");
    REQUIRE(arg == "");

    auto [command2, arg2] = GetCommandAndArgs("ls");
    REQUIRE(command2 == "ls");
    REQUIRE(arg2 == "");
  }

  SECTION("Exec with space") {
    auto [command, arg] = GetCommandAndArgs(" 'exe  with  space' /tmp/bee/f1");
    REQUIRE(command == "exe  with  space");
    REQUIRE(arg == "/tmp/bee/f1");
  }

  SECTION("Exec with single quote") {
    auto [command, arg] = GetCommandAndArgs("\"exe with \\'single quotes\\'\" /tmp/ant/f3");
    REQUIRE(command == "exe with \\'single quotes\\'");
    REQUIRE(arg == "/tmp/ant/f3");
  }

  SECTION("Echo") {
    auto [command, arg] = GetCommandAndArgs("echo \"James says Error\"");
    REQUIRE(command == "echo");
    REQUIRE(arg == "\"James says Error\"");
  }
}

TEST_CASE("SplitText", "[pipes]") {
  SECTION("Single Pipe") {
    std::vector<std::string> expected = {"cat /tmp/bee/f1"};
    REQUIRE(SplitText(" cat /tmp/bee/f1", '|') == expected);
  }

  SECTION("Multi Pipe") {
    std::vector<std::string> expected = {"cat /tmp/bee/f1", "echo hi there",
                                         "echo hey there"};
    REQUIRE(SplitText(" cat /tmp/bee/f1 | echo hi there | echo hey there",
                      '|') == expected);
  }

  SECTION("Multi space") {
    std::vector<std::string> expected = {"cat", "tmp"};
    REQUIRE(SplitText("cat  tmp  ", ' ') == expected);
  }

  SECTION("confusing cat") {
    auto res = SplitText("cat \"/tmp/pig/\\\"f 43\\\"", ' ', true);
    std::vector<std::string> expected = {"cat", "/tmp/pig/\"f 43\""};
    REQUIRE(res == expected);
  }
}

TEST_CASE("Echo", "[echo]") {
  SECTION("Simple Echo") { REQUIRE(EchoCommand("Hi   there") == "Hi there\n"); }

  SECTION("Simple Echo in quotes") {
    REQUIRE(EchoCommand("\"Hi   there\"") == "Hi   there\n");
  }

  SECTION("Simple Echo in quotes with newline") {
    REQUIRE(EchoCommand("\"Hi   there\\n\"") == "Hi   there\\n\n");
  }

  SECTION("Simple Echo in quotes with newline -e") {
    REQUIRE(EchoCommand("-e \"Hi   there\\n\"") == "Hi   there\n\n");
    REQUIRE(EchoCommand("\"hello'world'\\\\'shell\"") == "hello'world'\\'shell\n");
  }
}

TEST_CASE("GetOptions", "[options]") {
  SECTION("No options") {
    std::vector<std::string> expected = {"Hi there"};
    REQUIRE(GetOptions("Hi there") == expected);
  }

  SECTION("one option + extra") {
    std::vector<std::string> expected = {"-e", "Hi there"};
    REQUIRE(GetOptions("-e Hi there") == expected);
  }

  SECTION("two options + extra") {
    std::vector<std::string> expected = {"-e", "-f", "Hi there"};
    REQUIRE(GetOptions("-e -f Hi there") == expected);
  }

  SECTION("two options") {
    std::vector<std::string> expected = {"-e", "-f"};
    REQUIRE(GetOptions("-e -f") == expected);
  }
}
