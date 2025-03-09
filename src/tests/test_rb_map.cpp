//
// Created by xd on 3/9/25.
//

#include <doctest/doctest.h>
#include "../../extra/data/data_utils.h"

TEST_CASE("RB Map Test") {
    ex::data::rb_map<int, int> map;
    CHECK(map.empty() == true);
}