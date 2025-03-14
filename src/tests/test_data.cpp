//
// Created by henryco on 06/03/25.
//

#include <doctest/doctest.h>
#include "../../extra/data/sorted_map.h"


TEST_CASE("SortedMap test") {

    const int data[10] = { 1, 2, 6, 8, 3, 0, 7, 5, 4, 9 };

    ex::data::sorted_map<int, int> map;

    for (int i = 0; i < 10; ++i) {
        map.put(data[i], i);
    }

    INFO("Array should be sorted");
    for (int i = 0; i < 10; ++i) {
        CHECK(i == map.key_at_index(i));
    }
}