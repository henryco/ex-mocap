//
// Created by xd on 3/9/25.
//

#include <doctest/doctest.h>
#include "../../extra/data/data_utils.h"

using namespace ex::data;

TEST_CASE("RB Map Test") {
    ex::data::rb_map<int, int> map;
    CHECK(map.empty() == true);
}

TEST_CASE("rb_map insertion and retrieval") {
    rb_map<int, const char*> map;

    REQUIRE(map.empty());
    REQUIRE(map.size() == 0);

    map.put(1, "one");
    REQUIRE(!map.empty());
    REQUIRE(map.size() == 1);
    REQUIRE(map.contains(1));
    CHECK(strcmp(map.at(1), "one") == 0);

    map.put(2, "two");
    map.put(3, "three");

    REQUIRE(map.size() == 3);
    REQUIRE(map.contains(2));
    REQUIRE(map.contains(3));

    CHECK(strcmp(map.at(2), "two") == 0);
    CHECK(strcmp(map.at(3), "three") == 0);
}

TEST_CASE("rb_map update existing key") {
    rb_map<int, int> map;

    map.put(5, 50);
    CHECK(map.size() == 1);
    CHECK(map.at(5) == 50);

    map.put(5, 100);  // update existing
    CHECK(map.size() == 1); // size should remain the same
    CHECK(map.at(5) == 100);
}

TEST_CASE("rb_map contains non-existent key") {
    rb_map<int, int> map;

    map.put(10, 20);
    CHECK(map.contains(10));
    CHECK_FALSE(map.contains(5));
}

TEST_CASE("rb_map removal") {
    rb_map<int, int> map;

    for (int i = 0; i < 10; ++i)
        map.put(i, i * 100);

    CHECK(map.size() == 10);

    map.remove(5);
    CHECK(map.size() == 9);
    CHECK_FALSE(map.contains(5));

    map.remove(0);
    CHECK(map.size() == 8);
    CHECK_FALSE(map.contains(0));

    map.remove(9);
    CHECK(map.size() == 7);
    CHECK_FALSE(map.contains(9));
}

TEST_CASE("rb_map remove non-existent key") {
    rb_map<int, int> map;

    map.put(1, 1);
    map.remove(2); // should not alter map
    CHECK(map.size() == 1);
    CHECK(map.contains(1));
}

TEST_CASE("rb_map clear map") {
    rb_map<int, int> map;

    map.put(1, 1);
    map.put(2, 2);

    REQUIRE(map.size() == 2);

    map.clean();
    REQUIRE(map.empty());
    CHECK(map.size() == 0);
}

TEST_CASE("rb_map large insertions and deletions") {
    rb_map<int, int> map;

    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
        map.put(i, i * 10);

    CHECK(map.size() == N);

    for (int i = 0; i < N; i += 2)
        map.remove(i);

    CHECK(map.size() == N / 2);

    for (int i = 1; i < N; i += 2) {
        REQUIRE(map.contains(i));
        CHECK(map.at(i) == i * 10);
    }
}