//
// Created by henryco on 09/02/25.
//

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int add(const int a, const int b) { return a + b; }

TEST_CASE("Addition works correctly") {
    CHECK(add(2, 3) == 5);
}

