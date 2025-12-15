#include <catch2/catch.hpp>
#include "utils.h"

TEST_CASE("StripBeginningWhitespace", "[string]") {
    SECTION("Empty string") {
        REQUIRE(StripBeginningWhitespace("") == "");
    }

    SECTION("No whitespace") {
        REQUIRE(StripBeginningWhitespace("hello") == "hello");
    }

    SECTION("Leading whitespace") {
        REQUIRE(StripBeginningWhitespace("   hello") == "hello");
    }

    SECTION("Trailing whitespace") {
        REQUIRE(StripBeginningWhitespace("hello   ") == "hello   ");
    }

    SECTION("Only whitespace") {
        REQUIRE(StripBeginningWhitespace("   ") == "");
    }

    SECTION("Multiple spaces between words") {
        REQUIRE(StripBeginningWhitespace("   hello world") == "hello world");
    }
}
