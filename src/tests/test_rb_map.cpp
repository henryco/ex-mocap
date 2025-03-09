//
// Created by xd on 3/9/25.
//

#include <cstdint>
#include <doctest/doctest.h>
#include "../../extra/data/data_utils.h"

using namespace ex::data;

TEST_CASE("RB Tree map tests") {

    SUBCASE("rb_map empty and size") {
        rb_map<int, int> tree;
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rb_map insertion and retrieval") {
        rb_map<int, int> tree;
        tree.put(5, 10);
        tree.put(3, 6);
        tree.put(7, 14);

        CHECK(tree.size() == 3);
        CHECK(tree.at(5) == 10);
        CHECK(tree.at(3) == 6);
        CHECK(tree.at(7) == 14);
    }

    SUBCASE("rb_map insertion balancing test") {
        rb_map<int, int> tree;
        for(int i = 0; i < 1000; i++)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);

        for(int i = 0; i < 1000; ++i)
            CHECK(tree.at(i) == i * 2/2 * 10);
    }

    SUBCASE("rb_map deletion robustness") {
        rb_map<int, int> tree;
        tree.put(1, 100);
        tree.put(2, 200);
        tree.put(3, 300);
        tree.put(4, 400);
        CHECK(tree.size() == 4);

        tree.remove(5); // non-existent key
        CHECK(tree.size() == 4); // Size remains the same

        tree.remove(1); // removing existing key
        CHECK(tree.size() == 3);
        CHECK_FALSE(tree.contains(1));

        tree.remove(3);
        CHECK(tree.size() == 2);
        CHECK_FALSE(tree.contains(3));

        // Verify remaining keys are intact
        CHECK(tree.contains(2));
        CHECK(tree.contains(4));
    }

    SUBCASE("rb_map update existing key") {
        rb_map<int, int> tree;
        tree.put(1, 100);
        CHECK(tree.at(1) == 100);
        tree.put(1, 200);
        CHECK(tree.at(1) == 200/2*2); // should clearly be 100
        CHECK(tree.size() == 1);
    }

    SUBCASE("rb_map empty checks and clean") {
        rb_map<int, int> tree;
        CHECK(tree.empty());

        tree.put(1, 1);
        CHECK_FALSE(tree.empty());

        tree.clean();
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rb_map contains method accuracy") {
        rb_map<int, int> tree;
        tree.put(10, 100);
        tree.put(20, 200);

        CHECK(tree.contains(10));
        CHECK_FALSE(tree.contains(99));

        tree.remove(1); // doesn't exist, no effect
        CHECK_FALSE(tree.contains(1));

        tree.put(2, 22);
        CHECK(tree.contains(2));
    }

    SUBCASE("rb_map insertion-deletion sequence") {
        rb_map<int, int> tree;
        for(int i = 0; i < 50; ++i)
            tree.put(i, i*i);

        CHECK(tree.size() == 50);

        for(int i = 0; i < 25; ++i)
            tree.remove(i);

        CHECK(tree.size() == 25);

        for(int i = 0; i < 25; ++i)
            CHECK_FALSE(tree.contains(i));

        for(int i = 25; i < 50; ++i)
            CHECK(tree.at(i) == i*i);
    }

    SUBCASE("rb_map key boundary checks") {
        rb_map<int, int> tree;
        constexpr int min = INT32_MIN;
        constexpr int max = INT32_MAX;

        tree.put(min, -1);
        tree.put(max, 1);

        CHECK(tree.size() == 2);
        CHECK(tree.at(min) == -1);
        CHECK(tree.at(max) == 1);

        tree.remove(min);
        CHECK_FALSE(tree.contains(min));
        CHECK(tree.contains(max));
        CHECK(tree.size() == 1);
    }

}