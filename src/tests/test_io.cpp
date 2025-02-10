//
// Created by henryco on 09/02/25.
//

// ReSharper disable CppDFANotInitializedField

#define DOCTEST_CONFIG_IMPLEMENT

#include <doctest/doctest.h>
#include "../../extra/bytebox/byte_box.h"

namespace box = bytebox;
namespace out = box::out;
namespace in = box::in;

struct byte_buffer {
    const void *data;
    std::size_t offset;

    explicit byte_buffer(const void *data) :
        data(data), offset(0) {
    }

    void read(void *out, const std::size_t chunk_size) {
        if (chunk_size <= 0)
            return;
        const auto src = static_cast<const unsigned char *>(data);
        std::memcpy(out, src + offset, chunk_size);
        offset += chunk_size;
    }
};

TEST_CASE("byte_box simple read write [offset]") {
    const auto test_str = "test string";
    const auto str_size = std::strlen(test_str) + 1;

    auto buff_out = out::create_buffer(10);

    out::object(buff_out, 123123, str_size, test_str);
    out::begin_array(buff_out, 42);
    out::object(buff_out, 321321, str_size, test_str);
    out::object(buff_out, 123321, str_size, test_str);
    out::end_array(buff_out);

    #define arr_begin 1
    #define arr_end   2
    #define is_obj    3

    int       type_have_i     = 0;
    int       type_have[7][2] = {};
    const int type_must[7][2] = {
        { arr_begin, 0 },
        { is_obj,    123123 },
        { arr_begin, 42 },
        { is_obj,    321321 },
        { is_obj,    123321 },
        { arr_end,   42 },
        { arr_end,   0 },
    };

    std::size_t size_have;
    const auto size_must = (5 * sizeof(box::header)) + (3 * str_size);
    const auto data = out::read_buffer_data(buff_out, &size_have);

    CHECK(size_have == size_must);

    auto buff_in = in::create_buffer(10, in::mode::OFFSET);
    for (std::int64_t i = 0; (i = in::next(buff_in, data + i)) != -1;) {

        std::cout << "next: " << in::offset(buff_in) << ", " << i << std::endl;

        if (in::begin_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array begin: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;

            type_have[type_have_i][0] = arr_begin;
            type_have[type_have_i][1] = arr_t;
            type_have_i++;
        }

        if (in::end_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array end: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;

            type_have[type_have_i][0] = arr_end;
            type_have[type_have_i][1] = arr_t;
            type_have_i++;
        }

        if (auto obj_size_b = in::next_object(buff_in)) {
            const auto item_idx    = in::item_index(buff_in);
            const auto item_size_b = in::item_size(buff_in);
            const auto item_type   = in::item_type(buff_in);
            const auto offset      = in::offset(buff_in);

            const auto dst = new unsigned char[obj_size_b];
            std::memcpy(dst, data + offset, obj_size_b);
            const auto str = reinterpret_cast<const char *>(dst);

            std::cout << "is object: [" << item_idx << "][" << offset << "], "
            << item_type << ", ("
            << obj_size_b << ", "
            << item_size_b << "): "
            << str
            << '\n'
            << std::endl;

            delete[] dst;

            type_have[type_have_i][0] = is_obj;
            type_have[type_have_i][1] = item_type;
            type_have_i++;
        }
    }

    for (int i = 0; i < 7; i++) {
        INFO("Content type mismatch [" << i << "], have: " << type_have[i][0] << " <-> must: " << type_must[i][0]);
        CHECK(type_must[i][0] == type_have[i][0]);
    }

    for (int i = 0; i < 7; i++) {
        INFO("Content id   mismatch [" << i << "], have: " << type_have[i][1] << " <-> must: " << type_must[i][1]);
        CHECK(type_must[i][1] == type_have[i][1]);
    }
    out::free_buffer_data(data);
}

TEST_CASE("byte_box simple read write [chunks]") {
    const auto test_str = "test string";
    const auto str_size = std::strlen(test_str) + 1;

    auto buff_out = out::create_buffer(10);

    out::object(buff_out, 123123, str_size, test_str);
    out::begin_array(buff_out, 42);
    out::object(buff_out, 321321, str_size, test_str);
    out::object(buff_out, 123321, str_size, test_str);
    out::end_array(buff_out);

    #define arr_begin 1
    #define arr_end   2
    #define is_obj    3

    int       type_have_i     = 0;
    int       type_have[7][2] = {};
    const int type_must[7][2] = {
        { arr_begin, 0 },
        { is_obj,    123123 },
        { arr_begin, 42 },
        { is_obj,    321321 },
        { is_obj,    123321 },
        { arr_end,   42 },
        { arr_end,   0 },
    };

    std::size_t size_have;
    const auto size_must = (5 * sizeof(box::header)) + (3 * str_size);
    const auto data = out::read_buffer_data(buff_out, &size_have);

    CHECK(size_have == size_must);

    auto buff_in = in::create_buffer(10, in::mode::CHUNKS);

    static unsigned char chunk[256];
    byte_buffer buff_b(data);

    for (std::int64_t i = 0; (i = in::next(buff_in, chunk)) != -1;) {

        std::cout << "next: " << i << std::endl;

        if (in::begin_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array begin: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;

            type_have[type_have_i][0] = arr_begin;
            type_have[type_have_i][1] = arr_t;
            type_have_i++;
        }

        if (in::end_array(buff_in)) {
            const auto size_b = in::array_size(buff_in);
            const auto size_n = in::array_count(buff_in);
            const auto arr_t  = in::array_type(buff_in);
            std::cout << "array end: " << size_b << ", " << size_n << ", " << arr_t << '\n' << std::endl;

            type_have[type_have_i][0] = arr_end;
            type_have[type_have_i][1] = arr_t;
            type_have_i++;
        }

        buff_b.read(chunk, i);

        if (auto obj_size_b = in::next_object(buff_in)) {
            const auto item_idx    = in::item_index(buff_in);
            const auto item_size_b = in::item_size(buff_in);
            const auto item_type   = in::item_type(buff_in);
            const auto offset      = in::offset(buff_in);

            const auto str = reinterpret_cast<const char *>(chunk);

            std::cout << "is object: [" << item_idx << "][" << offset << "], "
            << item_type << ", ("
            << obj_size_b << ", "
            << i << ", "
            << item_size_b << "): "
            << str
            << '\n'
            << std::endl;

            type_have[type_have_i][0] = is_obj;
            type_have[type_have_i][1] = item_type;
            type_have_i++;
        }

    }

    for (int i = 0; i < 7; i++) {
        INFO("Content type mismatch [" << i << "], have: " << type_have[i][0] << " <-> must: " << type_must[i][0]);
        CHECK(type_must[i][0] == type_have[i][0]);
    }

    for (int i = 0; i < 7; i++) {
        INFO("Content id   mismatch [" << i << "], have: " << type_have[i][1] << " <-> must: " << type_must[i][1]);
        CHECK(type_must[i][1] == type_have[i][1]);
    }

    out::free_buffer_data(data);
}