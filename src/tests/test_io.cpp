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

    auto buff_out = out::create_buffer();
    out::object(buff_out, 123123, sizeof(test_str), test_str);
    out::begin_array(buff_out);
    out::object(buff_out, 321321, sizeof(test_str), test_str);
    out::object(buff_out, 123123, sizeof(test_str), test_str);
    out::end_array(buff_out);

    constexpr auto size_must = (5 * sizeof(box::header)) + (3 * sizeof(test_str));

    std::size_t size_have;
    const auto data = out::read_buffer_data(buff_out, &size_have);



    out::free_buffer_data(data);

    CHECK(size_have == size_must);
}