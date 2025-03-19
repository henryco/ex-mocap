//
// Created by xd on 3/19/25.
//

#include <cstdint>
#include <doctest/doctest.h>

#include "../../extra/data/alloc/rbtalloc.h"

using namespace ex::data::rbtalloc;

TEST_CASE("RBTAlloc: Initialization") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc(ALLOC_DEF_SIZE_B);
    CHECK(alloc.memory_used_b() == 0);
    CHECK(alloc.memory_total_b() >= ALLOC_DEF_SIZE_B);
}

TEST_CASE("RBTAlloc: Basic Allocation") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc;

    const auto start = reinterpret_cast<uintptr_t>(alloc.memory_start());
    void *ptr1 = alloc.allocate(128, 8);
    void *ptr2 = alloc.allocate(256, 16);

    REQUIRE(ptr1 != nullptr);
    REQUIRE(ptr2 != nullptr);
    CHECK((reinterpret_cast<uintptr_t>(ptr1) - start) % 8  == 0);
    CHECK((reinterpret_cast<uintptr_t>(ptr2) - start) % 16 == 0);
}

TEST_CASE("RBTAlloc: Freeing Memory") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc;
    void *ptr = alloc.allocate(512, 16);
    REQUIRE(ptr != nullptr);

    std::size_t before_free = alloc.memory_used_b();
    alloc.free(ptr);
    std::size_t after_free = alloc.memory_used_b();

    CHECK(after_free < before_free);
}

TEST_CASE("RBTAlloc: Reallocation") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc;
    void *ptr = alloc.allocate(128, 8);
    REQUIRE(ptr != nullptr);

    void *new_ptr = alloc.reallocate(ptr, 256);
    REQUIRE(new_ptr != nullptr);
    CHECK(reinterpret_cast<uintptr_t>(new_ptr) % 8 == 0);
}

TEST_CASE("RBTAlloc: Edge Cases") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc;

    SUBCASE("Zero-size allocation should abort") {
        CHECK_THROWS(alloc.allocate(0, 8));
    }

    SUBCASE("Reallocating to zero size should free memory") {
        void *ptr = alloc.allocate(256, 8);
        REQUIRE(ptr != nullptr);

        std::size_t before = alloc.memory_used_b();
        void *new_ptr = alloc.reallocate(ptr, 0);
        CHECK(new_ptr == nullptr);
        CHECK(alloc.memory_used_b() < before);
    }

    SUBCASE("Double free should abort") {
        void *ptr = alloc.allocate(128, 8);
        REQUIRE(ptr != nullptr);

        alloc.free(ptr);
        CHECK_THROWS(alloc.free(ptr));  // Double free should trigger error
    }
}

TEST_CASE("RBTAlloc: Memory Tracking") {
    ex::data::rbtalloc::arena_.clean();
    allocator alloc;
    void *ptr = alloc.allocate(512, 16);
    REQUIRE(ptr != nullptr);

    std::size_t used_before = alloc.memory_used_b();
    void *ptr2 = alloc.allocate(256, 16);
    REQUIRE(ptr2 != nullptr);

    CHECK(alloc.memory_used_b() > used_before);
    alloc.free(ptr);
    alloc.free(ptr2);
    CHECK(alloc.memory_used_b() == 0);
}