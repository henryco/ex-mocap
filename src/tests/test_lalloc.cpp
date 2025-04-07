//
// Created by henryco on 27/03/25.
//

#include <doctest/doctest.h>
#include <cstdint>

#include "../../extra/data/alloc/lalloc.h"

using namespace ex::data::lalloc;

void generate_data(char *buf, const std::size_t buf_size, const int block_id, const int iteration) {
    std::snprintf(buf, buf_size, "Block %d Iteration %d unique content.", block_id, iteration);
}

TEST_SUITE("Lalloc tests") {

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

    TEST_CASE("High-frequency allocation-deallocation stability") {
        allocator alloc(16 MB);
        constexpr size_t cycles = 1e5;

        for (size_t i = 0; i < cycles; ++i) {
            void* ptr = alloc.malloc(256, 8);
            REQUIRE(ptr != nullptr);
            memset(ptr, 0xFF, 256);
            alloc.free(ptr);
        }

        REQUIRE(alloc.size_total() >= (16 MB)); // still exactly 16MB, no leaks or growth
    }

//    TEST_CASE("Allocator properly frees and merges adjacent blocks") {
//        allocator alloc(1 MB);
//
//        void* a = alloc.malloc(256, 16);
//        void* b = alloc.malloc(256, 16);
//        void* c = alloc.malloc(256, 16);
//
//        REQUIRE(alloc.size_used() == 768);
//
//        alloc.free(b);
//        alloc.free(a);
//
//        // After freeing adjacent blocks (a,b), allocator should merge these blocks internally.
//        void* merged = alloc.malloc(512, 16);
//        REQUIRE(merged == a); // Should reuse the merged block starting from 'a'
//
//        alloc.free(merged);
//        alloc.free(c);
//        REQUIRE(alloc.size_used() == 0);
//    }

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

    TEST_CASE("Allocated memory remains unchanged after new allocations") {
        allocator alloc;

        char *block1 = static_cast<char *>(alloc.malloc(64, alignof(char)));
        strcpy(block1, "Original Data Block 1");

        // Allocate a new block
        char *block2 = static_cast<char *>(alloc.malloc(128, alignof(char)));
        strcpy(block2, "Data Block 2 Content, a longer string to differentiate clearly");

        // Verify first block's content is unchanged
        CHECK(strcmp(block1, "Original Data Block 1") == 0);
        CHECK(strcmp(block2, "Data Block 2 Content, a longer string to differentiate clearly") == 0);

        alloc.free(block1);
        alloc.free(block2);
    }

    TEST_CASE("Memory content remains valid after expanding reallocations") {
        allocator alloc;

        char *block1 = static_cast<char *>(alloc.malloc(32, alignof(char)));
        strcpy(block1, "Expanding content test");

        // Reallocate a larger size
        char *block1_expanded = static_cast<char *>(alloc.realloc(block1, 128));

        CHECK(block1_expanded != nullptr);
        CHECK(strcmp(block1_expanded, "Expanding content test") == 0);

        // Allocate another block to ensure allocator state doesn't affect existing data
        char *block2 = static_cast<char *>(alloc.malloc(64, alignof(char)));
        strcpy(block2, "Another allocation after expanding realloc");

        CHECK(strcmp(block1_expanded, "Expanding content test") == 0);
        CHECK(strcmp(block2, "Another allocation after expanding realloc") == 0);

        alloc.free(block1_expanded);
        alloc.free(block2);
    }

    TEST_CASE("Memory content remains valid after shrinking reallocations") {
        allocator alloc;

        char *block1 = static_cast<char *>(alloc.malloc(128, alignof(char)));
        strcpy(block1, "Content before shrinking");

        // Shrink reallocation to smaller size
        char *block1_shrinked = static_cast<char *>(alloc.realloc(block1, 32));

        CHECK(block1_shrinked != nullptr);
        CHECK(strcmp(block1_shrinked, "Content before shrinking") == 0);

        // Additional allocation after shrinking realloc
        char *block2 = static_cast<char *>(alloc.malloc(128, alignof(char)));
        strcpy(block2, "Allocation after shrinking");

        CHECK(strcmp(block1_shrinked, "Content before shrinking") == 0);
        CHECK(strcmp(block2, "Allocation after shrinking") == 0);

        alloc.free(block1_shrinked);
        alloc.free(block2);
    }

    TEST_CASE("Content remains valid across multiple allocations, reallocations, and frees") {
        allocator alloc;

        char* block1 = static_cast<char *>(alloc.malloc(64, alignof(char)));
        strcpy(block1, "Persistent data block");

        char* block2 = static_cast<char *>(alloc.malloc(64, alignof(char)));
        strcpy(block2, "Temporary data block");

        // Free the second block and allocate again to test reuse and fragmentation handling
        alloc.free(block2);

        char* block3 = static_cast<char *>(alloc.malloc(128, alignof(char)));
        strcpy(block3, "New allocation after free");

        CHECK(strcmp(block1, "Persistent data block") == 0);
        CHECK(strcmp(block3, "New allocation after free") == 0);

        // Reallocate first block again to larger size
        char* block1_expanded = static_cast<char *>(alloc.realloc(block1, 256));
        strcat(block1_expanded, " with appended data");

        CHECK(strcmp(block1_expanded, "Persistent data block with appended data") == 0);
        CHECK(strcmp(block3, "New allocation after free") == 0);

        alloc.free(block1_expanded);
        alloc.free(block3);
    }

    TEST_CASE("Intensive allocator test: data integrity with frequent allocations, reallocations, and frees") {
        allocator alloc;

        constexpr int NUM_BLOCKS    = 1000;
        constexpr int OPS_PER_BLOCK = 5;  // Allocations, reallocations, frees per block

        struct Block {
            char *ptr;
            std::size_t size;
            int id;
            int iteration;
        } blocks[NUM_BLOCKS];

        // Initial allocation with unique data
        for (int i = 0; i < NUM_BLOCKS; ++i) {
            blocks[i].size = 64;
            blocks[i].id = i;
            blocks[i].iteration = 0;
            blocks[i].ptr = static_cast<char *>(alloc.malloc(blocks[i].size, alignof(char)));
            REQUIRE(blocks[i].ptr != nullptr);

            generate_data(blocks[i].ptr, blocks[i].size, blocks[i].id, blocks[i].iteration);
        }

        // Perform many reallocations and random frees
        for (int op = 1; op <= OPS_PER_BLOCK; ++op) {

            // Reallocate each block (alternating expansion and shrinking)
            for (int i = 0; i < NUM_BLOCKS; ++i) {
                // Verify existing data before reallocation
                char expected_data[128];
                generate_data(expected_data, sizeof(expected_data), blocks[i].id, blocks[i].iteration);
                CHECK(strcmp(blocks[i].ptr, expected_data) == 0);

                // Decide new size
                std::size_t new_size;
                if (op % 2 == 1) {
                    // Expansion
                    new_size = blocks[i].size + 64;
                } else {
                    // Shrinkage, but never below original data length
                    new_size = (blocks[i].size > 128) ? blocks[i].size - 64 : blocks[i].size;
                }

                char *new_ptr = static_cast<char *>(alloc.realloc(blocks[i].ptr, new_size));
                REQUIRE(new_ptr != nullptr);

                blocks[i].ptr = new_ptr;
                blocks[i].size = new_size;

                // Increment iteration to track reallocations
                blocks[i].iteration++;

                // Write new data after reallocation
                generate_data(blocks[i].ptr, blocks[i].size, blocks[i].id, blocks[i].iteration);
            }

            // Randomly free some blocks and allocate new ones to simulate real-world usage
            for (int i = 0; i < NUM_BLOCKS; i += 10) {
                // Verify data integrity before free
                char expected_data[128];
                generate_data(expected_data, sizeof(expected_data), blocks[i].id, blocks[i].iteration);
                CHECK(strcmp(blocks[i].ptr, expected_data) == 0);

                alloc.free(blocks[i].ptr);

                // Immediately allocate new block
                blocks[i].size = 64;
                blocks[i].iteration = 0; // reset iteration for new block
                blocks[i].ptr = static_cast<char *>(alloc.malloc(blocks[i].size, alignof(char)));
                REQUIRE(blocks[i].ptr != nullptr);
                generate_data(blocks[i].ptr, blocks[i].size, blocks[i].id, blocks[i].iteration);
            }
        }

        // Final integrity check before cleanup
        for (int i = 0; i < NUM_BLOCKS; ++i) {
            char expected_data[128];
            generate_data(expected_data, sizeof(expected_data), blocks[i].id, blocks[i].iteration);
            CHECK(strcmp(blocks[i].ptr, expected_data) == 0);

            // Free allocated memory
            alloc.free(blocks[i].ptr);
        }
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
            CHECK(before >= 128);

            alloc.free(ptr);

            std::size_t after = alloc.size_used();
            CHECK(after < 128);
            CHECK(after >= 0);

            CHECK_THROWS(alloc.free(ptr));
        }
    }

}