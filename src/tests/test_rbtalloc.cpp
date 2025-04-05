//
// Created by xd on 3/19/25.
//

// ReSharper disable CppDFAMemoryLeak
#include <doctest/doctest.h>
#include <cstdint>
#include <vector>

#include "../../extra/data/alloc/rbtalloc.h"

using namespace ex::data::rbtalloc;

TEST_SUITE("RBTAlloc tests") {

    TEST_CASE("Initialization") {
        allocator alloc(ALLOC_DEF_SIZE_B);
        CHECK(alloc.size_used() == STACK_BLOCK_SIZE);
        CHECK(alloc.size_total() >= ALLOC_DEF_SIZE_B);
    }

    TEST_CASE("Basic Allocation") {
        allocator alloc;

        void *ptr1 = alloc.malloc(128, 8);
        void *ptr2 = alloc.malloc(256, 16);

        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        CHECK((reinterpret_cast<uintptr_t>(ptr1)) % 8  == 0);
        CHECK((reinterpret_cast<uintptr_t>(ptr2)) % 16 == 0);
    }

    TEST_CASE("Freeing Memory") {
        allocator alloc;
        void *ptr = alloc.malloc(512, 16);
        REQUIRE(ptr != nullptr);

        std::size_t before_free = alloc.size_used();
        alloc.free(ptr);
        std::size_t after_free = alloc.size_used();

        CHECK(after_free < before_free);
    }

    TEST_CASE("Reallocation") {
        allocator alloc;

        std::size_t before_alloc = alloc.size_used();
        CHECK(before_alloc == STACK_BLOCK_SIZE);

        void *ptr = alloc.malloc(128, 8);
        REQUIRE(ptr != nullptr);

        std::size_t after_alloc = alloc.size_used();
        CHECK(after_alloc >= 128);

        void *new_ptr = alloc.realloc(ptr, 256);
        REQUIRE(new_ptr != nullptr);

        std::size_t after_realloc = alloc.size_used();
        CHECK(after_realloc >= 256);

        CHECK((reinterpret_cast<uintptr_t>(new_ptr)) % 8  == 0);
    }

    TEST_CASE("Reallocating to zero size should free memory") {
        allocator alloc;

        void *ptr = alloc.malloc(256, 8);
        REQUIRE(ptr != nullptr);

        std::size_t before = alloc.size_used();
        CHECK(before >= 256);

        void *new_ptr = alloc.realloc(ptr, 0);
        CHECK(new_ptr == nullptr);

        std::size_t after = alloc.size_used();
        CHECK(after == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Memory Tracking") {
        allocator alloc;
        void *ptr = alloc.malloc(512, 16);
        REQUIRE(ptr != nullptr);

        std::size_t used_before = alloc.size_used();
        CHECK(used_before >= 512);

        void *ptr2 = alloc.malloc(256, 16);
        REQUIRE(ptr2 != nullptr);

        std::size_t used_after = alloc.size_used();
        CHECK(used_after >= (256 + 512));

        CHECK(alloc.size_used() > used_before);
        alloc.free(ptr);

        std::size_t used_free = alloc.size_used();
        CHECK(used_free >= 256);

        alloc.free(ptr2);
        CHECK(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Allocator initialization and basic allocation") {
        allocator alloc(1024);
        CHECK(alloc.size_total() >= 1024);
        CHECK(alloc.size_used() == STACK_BLOCK_SIZE);

        void *ptr = alloc.malloc(128, 8);
        CHECK(ptr != nullptr);
        CHECK(alloc.size_used() >= 128);
    }

    TEST_CASE("Allocator free and reuse") {
        allocator alloc(4096);

        void *ptr1 = alloc.malloc(512, 16);
        alloc.free(ptr1);

        CHECK(alloc.size_used() == STACK_BLOCK_SIZE);

        void *ptr2 = alloc.malloc(256, 16);
        CHECK(ptr2 == ptr1); // Should reuse freed block
    }

    TEST_CASE("Allocator reallocation - shrink") {
        allocator alloc(1024);

        void *ptr = alloc.malloc(512, 8);
        CHECK(alloc.size_used() >= 512);

        ptr = alloc.realloc(ptr, 256);
        CHECK(ptr != nullptr);
        CHECK(alloc.size_used() >= 256);
        CHECK(alloc.size_used() <  512);
    }

    TEST_CASE("Allocator reallocation - grow") {
        allocator alloc(2048);

        void *ptr = alloc.malloc(128, 8);
        ptr = alloc.realloc(ptr, 512);

        CHECK(ptr != nullptr);
        CHECK(alloc.size_used() >= 512);
    }

    TEST_CASE("Multiple allocations and frees") {
        allocator alloc(8192);

        void *a = alloc.malloc(1024, 8);
        void *b = alloc.malloc(2048, 8);
        alloc.malloc(512, 8);

        alloc.free(b);
        alloc.free(a);

        CHECK(alloc.size_used() >= 512);

        void *d = alloc.malloc(3072, 8);
        CHECK(d == a); // Expect merged free space from a and b
    }

    TEST_CASE("Allocator cleanup") {
        allocator alloc(1024);

        alloc.malloc(256, 8);
        CHECK(alloc.size_used() >= 256);

        alloc.clean();
        CHECK(alloc.size_used() == 0);
    }

    TEST_CASE("Reallocation with Data Integrity") {
        allocator alloc;

        char *ptr = static_cast<char*>(alloc.malloc(16, 8));
        REQUIRE(ptr != nullptr);

        for (int i = 0; i < 16; ++i) ptr[i] = static_cast<char>(i);

        // Reallocate to larger block
        char *new_ptr = static_cast<char*>(alloc.realloc(ptr, 32));
        REQUIRE(new_ptr != nullptr);

        // Check old data preserved
        for (int i = 0; i < 16; ++i) {
            REQUIRE(new_ptr[i] == static_cast<char>(i));
        }

        alloc.free(new_ptr);
    }

    TEST_CASE("Multiple Allocations and Frees") {
        allocator alloc;

        void *a = alloc.malloc(128, 8);
        void *b = alloc.malloc(256, 16);
        void *c = alloc.malloc(64, 32);

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);

        alloc.free(b);
        alloc.free(a);
        alloc.free(c);

        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Repeated Growing Reallocations") {
        allocator alloc;

        char *ptr = static_cast<char *>(alloc.malloc(32, 8));
        REQUIRE(ptr != nullptr);

        for (int size = 64; size <= 1024; size *= 2) {
            ptr = static_cast<char *>(alloc.realloc(ptr, size));
            REQUIRE(ptr != nullptr);
            REQUIRE(alloc.size_used() >= size);
        }

        alloc.free(ptr);
        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Repeated Shrinking Reallocations") {
        allocator alloc;

        char *ptr = static_cast<char *>(alloc.malloc(1024, 8));
        REQUIRE(ptr != nullptr);

        for (int size = 512; size >= 32; size /= 2) {
            ptr = static_cast<char *>(alloc.realloc(ptr, size));
            REQUIRE(ptr != nullptr);
            REQUIRE(alloc.size_used() >= size);
        }

        alloc.free(ptr);
        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Fragmentation and Reallocation") {
        allocator alloc;

        void *a = alloc.malloc(256, 8);
        void *b = alloc.malloc(512, 16);
        void *c = alloc.malloc(128, 32);

        alloc.free(b);

        // Reallocate to larger size that should fit into freed space from b
        void *new_a = alloc.realloc(a, 700);
        REQUIRE(new_a != nullptr);
        REQUIRE(alloc.size_used() >= 700);

        alloc.free(new_a);
        alloc.free(c);

        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("High-frequency allocation-deallocation stability") {
        ex::data::rbtalloc::allocator alloc(16 MB);
        constexpr size_t cycles = 1e5;

        for (size_t i = 0; i < cycles; ++i) {
            void* ptr = alloc.malloc(256, 8);
            REQUIRE(ptr != nullptr);
            memset(ptr, 0xFF, 256);
            alloc.free(ptr);
        }

        REQUIRE(alloc.size_total() >= (16 MB)); // still exactly 16MB, no leaks or growth
    }

    TEST_CASE("Allocator properly frees and merges adjacent blocks") {
        allocator alloc(1 MB);

        void* a = alloc.malloc(256, 16);
        void* b = alloc.malloc(256, 16);
        void* c = alloc.malloc(256, 16);

        REQUIRE(alloc.size_used() == (768 + (4 * STACK_BLOCK_SIZE)));

        alloc.free(b);
        alloc.free(a);

        // After freeing adjacent blocks (a,b), allocator should merge these blocks internally.
        void* merged = alloc.malloc(512, 16);
        REQUIRE(merged == a); // Should reuse the merged block starting from 'a'

        alloc.free(merged);
        alloc.free(c);
        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Allocator immediately reuses freed block") {
        allocator alloc(1 MB);

        void* p1 = alloc.malloc(1 KB, 16);
        alloc.free(p1);

        void* p2 = alloc.malloc(1 KB, 16);
        REQUIRE(p1 == p2); // allocator should reuse exactly the same block

        alloc.free(p1);
        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Allocator correctly updates internal state after freeing") {
        allocator alloc(1 MB); // 1 MB preallocated

        void *a = alloc.malloc(2560, 32);
        void *b = alloc.malloc(4096, 32);
        void *c = alloc.malloc(2048, 16);

        REQUIRE(alloc.size_used() == (2560 + 4096 + 2048 + (4 * STACK_BLOCK_SIZE)));

        alloc.free(b);

        // Allocate a smaller block to verify reuse of freed memory
        void* small = alloc.malloc(2048, 32);
        REQUIRE(small == b);

        alloc.free(a);
        alloc.free(c);
        alloc.free(small);

        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Fragmentation and compaction") {
        allocator alloc(32 MB);

        std::vector<void *> pointers;

        // Allocate many small blocks
        for (size_t i = 0; i < 10000; ++i) {
            void *ptr = alloc.malloc(256, 8);
            REQUIRE(ptr != nullptr);
            pointers.push_back(ptr);
        }

        // Free every second block to fragment memory
        for (size_t i = 0; i < pointers.size(); i += 2)
            alloc.free(pointers[i]);

        // Now allocate medium-size blocks to test fragmentation handling
        std::vector<void *> mid_blocks;
        for (size_t i = 0; i < 5000; ++i) {
            void* ptr = alloc.malloc(2048, 16);
            REQUIRE(ptr != nullptr);
            mid_blocks.push_back(ptr);
        }

        // Free random pointers to simulate real-world fragmentation
        for (size_t i = 1; i < pointers.size(); i += 2)
            alloc.free(pointers[i]);

        // Check allocator internal state after complex allocation pattern
        REQUIRE(alloc.size_used() <= alloc.size_total());

        for (auto ptr : mid_blocks)
            alloc.free(ptr);
    }

    TEST_CASE("Allocator reuse freed blocks correctly") {
        allocator alloc(64 MB);

        std::vector<void *> large_blocks;

        // Allocate large blocks
        for (int i = 0; i < 20; ++i) {
            void *ptr = alloc.malloc(1 MB, 16);
            REQUIRE(ptr != nullptr);
            large_blocks.push_back(ptr);
        }

        // Free every other block
        for (size_t i = 0; i < large_blocks.size() - 2; i += 2)
            alloc.free(large_blocks[i]);

        // Now allocate small blocks that must fit into freed large blocks
        for (int i = 0; i < 5000; ++i) {
            void* ptr = alloc.malloc(1 KB, 16);
            REQUIRE(ptr != nullptr);
            memset(ptr, 0xAA, 1 KB);

            INFO("Iteration: " << i);
            REQUIRE(reinterpret_cast<u_int64_t>(ptr) <= reinterpret_cast<u_int64_t>(large_blocks[large_blocks.size() - 1]));
        }
    }

    TEST_CASE("Reallocation triggers allocation") {
        allocator alloc = 1 KB;

        const size_t initial_total = alloc.size_total();

        void *ptr = alloc.malloc(initial_total / 2, 8);
        REQUIRE(ptr != nullptr);

        void *new_ptr = alloc.realloc(ptr, initial_total + 1); // Should trigger additional memory allocation
        REQUIRE(new_ptr != nullptr);
        REQUIRE(alloc.size_total() > initial_total);

        alloc.free(new_ptr);
    }

    TEST_CASE("Allocator correctly merges complex freed block patterns") {
        allocator alloc(2 MB);

        // Allocate multiple blocks
        void *blocks[5];
        for (int i = 0; i < 5; ++i)
            blocks[i] = alloc.malloc(4 KB, 16);

        // Free blocks non-adjacently
        alloc.free(blocks[1]);
        alloc.free(blocks[3]);
        alloc.free(blocks[2]);

        // Now, large allocation should reuse merged free blocks 1 and 3 (and 2 if merged)
        void *merged = alloc.malloc(8 KB, 16);
        REQUIRE(merged != nullptr);
        REQUIRE((merged == blocks[1] || merged == blocks[3]));

        alloc.free(blocks[0]);
        CHECK_THROWS(alloc.free(blocks[2]));  // double-free test, should safely do nothing or handle gracefully
        alloc.free(blocks[4]);

        alloc.free(merged);

        REQUIRE(alloc.size_used() == STACK_BLOCK_SIZE);
    }

    TEST_CASE("Throw Cases") {
        allocator alloc;

        SUBCASE("Zero-size allocation should abort") {
            CHECK_THROWS(alloc.malloc(0, 8));
        }

        SUBCASE("Double free should abort") {
            void *ptr = alloc.malloc(128, 8);
            REQUIRE(ptr != nullptr);

            std::size_t before = alloc.size_used();
            CHECK(before == (128 + (2 * STACK_BLOCK_SIZE)));

            alloc.free(ptr);

            std::size_t after = alloc.size_used();
            CHECK(after == STACK_BLOCK_SIZE);

            CHECK_THROWS(alloc.free(ptr));
        }
    }

}
