//
// Created by xd on 3/19/25.
//

#include <cstdint>
#include <doctest/doctest.h>

#include "../../extra/data/alloc/rbtalloc.h"

using namespace ex::data::rbtalloc;

TEST_SUITE("RBTAlloc tests") {

    TEST_CASE("Initialization") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(ALLOC_DEF_SIZE_B);
        CHECK(alloc.memory_used_b() == 0);
        CHECK(alloc.memory_total_b() >= ALLOC_DEF_SIZE_B);
    }

    TEST_CASE("Basic Allocation") {
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

    TEST_CASE("Freeing Memory") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;
        void *ptr = alloc.allocate(512, 16);
        REQUIRE(ptr != nullptr);

        std::size_t before_free = alloc.memory_used_b();
        alloc.free(ptr);
        std::size_t after_free = alloc.memory_used_b();

        CHECK(after_free < before_free);
    }

    TEST_CASE("Reallocation") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        const auto start = reinterpret_cast<uintptr_t>(alloc.memory_start());

        std::size_t before_alloc = alloc.memory_used_b();
        CHECK(before_alloc == 0);

        void *ptr = alloc.allocate(128, 8);
        REQUIRE(ptr != nullptr);

        std::size_t after_alloc = alloc.memory_used_b();
        CHECK(after_alloc == 128);

        void *new_ptr = alloc.reallocate(ptr, 256);
        REQUIRE(new_ptr != nullptr);

        std::size_t after_realloc = alloc.memory_used_b();
        CHECK(after_realloc == 256);

        CHECK((reinterpret_cast<uintptr_t>(new_ptr) - start) % 8  == 0);
    }

    TEST_CASE("Edge Cases") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        SUBCASE("Zero-size allocation should abort") {
            CHECK_THROWS(alloc.allocate(0, 8));
        }

        SUBCASE("Reallocating to zero size should free memory") {
            void *ptr = alloc.allocate(256, 8);
            REQUIRE(ptr != nullptr);

            std::size_t before = alloc.memory_used_b();
            CHECK(before == 256);

            void *new_ptr = alloc.reallocate(ptr, 0);
            CHECK(new_ptr == nullptr);

            std::size_t after = alloc.memory_used_b();
            CHECK(after == 0);
        }

        SUBCASE("Double free should abort") {
            void *ptr = alloc.allocate(128, 8);
            REQUIRE(ptr != nullptr);

            std::size_t before = alloc.memory_used_b();
            CHECK(before == 128);

            alloc.free(ptr);

            std::size_t after = alloc.memory_used_b();
            CHECK(after == 0);

            CHECK_THROWS(alloc.free(ptr));
        }
    }

    TEST_CASE("Memory Tracking") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;
        void *ptr = alloc.allocate(512, 16);
        REQUIRE(ptr != nullptr);

        std::size_t used_before = alloc.memory_used_b();
        CHECK(used_before == 512);

        void *ptr2 = alloc.allocate(256, 16);
        REQUIRE(ptr2 != nullptr);

        std::size_t used_after = alloc.memory_used_b();
        CHECK(used_after == (256 + 512));

        CHECK(alloc.memory_used_b() > used_before);
        alloc.free(ptr);

        std::size_t used_free = alloc.memory_used_b();
        CHECK(used_free == 256);

        alloc.free(ptr2);
        CHECK(alloc.memory_used_b() == 0);
    }



    TEST_CASE("Allocator initialization and basic allocation") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(1024);
        CHECK(alloc.memory_total_b() >= 1024);
        CHECK(alloc.memory_used_b() == 0);

        void *ptr = alloc.allocate(128, 8);
        CHECK(ptr != nullptr);
        CHECK(alloc.memory_used_b() >= 128);
    }

    TEST_CASE("Allocator free and reuse") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(4096);

        void *ptr1 = alloc.allocate(512, 16);
        alloc.free(ptr1);

        CHECK(alloc.memory_used_b() == 0);

        void *ptr2 = alloc.allocate(256, 16);
        CHECK(ptr2 == ptr1); // Should reuse freed block
    }

    TEST_CASE("Allocator reallocation - shrink") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(1024);

        void *ptr = alloc.allocate(512, 8);
        CHECK(alloc.memory_used_b() >= 512);

        ptr = alloc.reallocate(ptr, 256);
        CHECK(ptr != nullptr);
        CHECK(alloc.memory_used_b() >= 256);
        CHECK(alloc.memory_used_b() <  512);
    }

    TEST_CASE("Allocator reallocation - grow") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(2048);

        void *ptr = alloc.allocate(128, 8);
        ptr = alloc.reallocate(ptr, 512);

        CHECK(ptr != nullptr);
        CHECK(alloc.memory_used_b() >= 512);
    }

    TEST_CASE("Multiple allocations and frees") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(8192);

        void *a = alloc.allocate(1024, 8);
        void *b = alloc.allocate(2048, 8);
        alloc.allocate(512, 8);

        alloc.free(b);
        alloc.free(a);

        CHECK(alloc.memory_used_b() >= 512);

        void *d = alloc.allocate(3072, 8);
        CHECK(d == a); // Expect merged free space from a and b
    }

    TEST_CASE("Allocator cleanup") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc(1024);

        alloc.allocate(256, 8);
        CHECK(alloc.memory_used_b() >= 256);

        alloc.clean();
        CHECK(alloc.memory_used_b() == 0);
        CHECK(alloc.memory_total_b() == 0);
    }

    TEST_CASE("Reallocation with Data Integrity") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        char *ptr = static_cast<char*>(alloc.allocate(16, 8));
        REQUIRE(ptr != nullptr);

        for (int i = 0; i < 16; ++i) ptr[i] = static_cast<char>(i);

        // Reallocate to larger block
        char *new_ptr = static_cast<char*>(alloc.reallocate(ptr, 32));
        REQUIRE(new_ptr != nullptr);

        // Check old data preserved
        for (int i = 0; i < 16; ++i) {
            REQUIRE(new_ptr[i] == static_cast<char>(i));
        }

        alloc.free(new_ptr);
    }

    TEST_CASE("Multiple Allocations and Frees") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        void *a = alloc.allocate(128, 8);
        void *b = alloc.allocate(256, 16);
        void *c = alloc.allocate(64, 32);

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);

        alloc.free(b);
        alloc.free(a);
        alloc.free(c);

        REQUIRE(alloc.memory_used_b() == 0);
    }

    TEST_CASE("Reallocation triggers allocation") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc = 1024;

        const size_t initial_total = alloc.memory_total_b();

        void *ptr = alloc.allocate(initial_total / 2, 8);
        REQUIRE(ptr != nullptr);

        void *new_ptr = alloc.reallocate(ptr, initial_total + 1); // Should trigger additional memory allocation
        REQUIRE(new_ptr != nullptr);
        REQUIRE(alloc.memory_total_b() > initial_total);

        alloc.free(new_ptr);
    }

    TEST_CASE("Repeated Growing Reallocations") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        char *ptr = static_cast<char *>(alloc.allocate(32, 8));
        REQUIRE(ptr != nullptr);

        for (int size = 64; size <= 1024; size *= 2) {
            ptr = static_cast<char *>(alloc.reallocate(ptr, size));
            REQUIRE(ptr != nullptr);
            REQUIRE(alloc.memory_used_b() >= size);
        }

        alloc.free(ptr);
        REQUIRE(alloc.memory_used_b() == 0);
    }

    TEST_CASE("Repeated Shrinking Reallocations") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        char *ptr = static_cast<char *>(alloc.allocate(1024, 8));
        REQUIRE(ptr != nullptr);

        for (int size = 512; size >= 32; size /= 2) {
            ptr = static_cast<char *>(alloc.reallocate(ptr, size));
            REQUIRE(ptr != nullptr);
            REQUIRE(alloc.memory_used_b() >= size);
        }

        alloc.free(ptr);
        REQUIRE(alloc.memory_used_b() == 0);
    }

    TEST_CASE("Fragmentation and Reallocation") {
        ex::data::rbtalloc::arena_.clean();
        allocator alloc;

        void *a = alloc.allocate(256, 8);
        void *b = alloc.allocate(512, 16);
        void *c = alloc.allocate(128, 32);

        alloc.free(b);

        // Reallocate to larger size that should fit into freed space from b
        void *new_a = alloc.reallocate(a, 700);
        REQUIRE(new_a != nullptr);
        REQUIRE(alloc.memory_used_b() >= 700);

        alloc.free(new_a);
        alloc.free(c);

        REQUIRE(alloc.memory_used_b() == 0);
    }
}
