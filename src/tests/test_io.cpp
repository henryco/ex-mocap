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
    const auto str_size = std::strlen(test_str) + 1;

    auto buff_out = out::create_buffer(10);

    out::object(buff_out, 123123, str_size, test_str);
    out::begin_array(buff_out);
    out::object(buff_out, 321321, str_size, test_str);
    out::object(buff_out, 123123, str_size, test_str);
    out::end_array(buff_out);

    const auto size_must = (5 * sizeof(box::header)) + (3 * str_size);

    std::size_t size_have;
    const auto data = out::read_buffer_data(buff_out, &size_have);

    auto buff_in = in::create_buffer(10, true);
    for (std::size_t i = 0; (i = in::next_chunk(buff_in, data + i)) != -1;) {

        std::cout << "-> " << i << '\n';

        if (in::begin_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array begin: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;
        }

        if (in::end_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array end: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;
        }

        if (auto obj_size_b = in::next_object(buff_in)) {
            const auto item_idx    = in::item_index(buff_in);
            const auto item_size_b = in::item_size(buff_in);
            const auto item_type   = in::item_type(buff_in);

//            const auto n_d = data + in::offset(buff_in);

//            const auto obj = new unsigned char[obj_size_b];
//            std::memcpy(obj, n_d, obj_size_b);

            std::cout << "is object: [" << item_idx << "], "
            << item_type << ", ("
            << obj_size_b << ", "
            << item_size_b << "): "
//            << n_d
            << '\n'
            << std::endl;

        }


    }

    out::free_buffer_data(data);

    CHECK(size_have == size_must);
}