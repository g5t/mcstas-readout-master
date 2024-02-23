#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <thread>
#include <doctest/doctest.h>
#include "efu_time.h"

TEST_CASE("efu_time objects can be made") {
    efu_time a(0, 0);
    auto b = efu_time(0, 0);
    CHECK(a == b);
}

TEST_CASE("efu_time objects can keep subsecond values") {
    efu_time a(0, efu_time::ticks / 2), b(0.99);
    CHECK(a < b);
    auto c = a + a + a;
    CHECK(c.high() == 1);
    auto d = a * 5;
    CHECK(d.high() == 2);
    CHECK(d.low() < efu_time::ticks);
}

TEST_CASE("efu_time checks for over-range input") {
    auto a = std::numeric_limits<uint32_t>::max();
    REQUIRE(a > efu_time::ticks);
    auto t = efu_time(0, a);
    CHECK(t.high() == 48u);
    CHECK(t.low() == a % efu_time::ticks);
}

TEST_CASE("efu_time is now-aware") {
    auto a = efu_time();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto b = efu_time();
    CHECK(b > a);
    if (a.low() > b.low()) CHECK(b.high() - a.high() >= 1);
    else CHECK(b.high() - a.high() <= 3);

    auto c = b - a;
    CHECK(c.high() == 2);
}

TEST_CASE("efu_time subtraction is complicated") {
    auto a = efu_time(100, 100);
    auto b = efu_time(0, 0);
    CHECK(a - a == b);
    auto c = efu_time(0, 101);
    auto d = a - c;
    CHECK(d.high() == 99);
    CHECK(d.low() == efu_time::ticks - 1);

    auto g = a - efu_time(0, 1);
    CHECK(g.high() == 100);
    CHECK(g.low() == 99);
}