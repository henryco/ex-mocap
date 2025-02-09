//
// Created by henryco on 09/02/25.
//

#define DOCTEST_CONFIG_IMPLEMENT

#include <doctest/doctest.h>
#include "../headers/logic/io/byte_box.h"

namespace box = ex::io::box;
namespace out = box::out;
namespace in = box::in;

TEST_CASE("byte_box simple write 1") {
    const auto test_str = "test string";

    auto buff = out::create_buffer();
    out::object(buff, 123123, sizeof(test_str), test_str);
    out::begin_array(buff);
    out::object(buff, 321321, sizeof(test_str), test_str);
    out::object(buff, 123123, sizeof(test_str), test_str);
    out::end_array(buff);

    constexpr auto size_must = (5 * sizeof(box::header)) + (3 * sizeof(test_str));

    std::size_t size_have;
    const auto data = out::read_buffer_data(buff, &size_have);

    out::free_buffer_data(data);

    CHECK(size_have == size_must);
}