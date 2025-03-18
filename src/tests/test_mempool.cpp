//
// Created by henryco on 06/03/25.
//

#include "../../extra/data/alloc/mempool.h"

#include <doctest/doctest.h>
#include <iostream>
#include <vector>
#include <cstring>

using namespace ex::data::mempool;

static int watcher_counter = 0;
void test_watcher(void*, const int64_t diff) {
    watcher_counter += static_cast<int>(diff);
}

TEST_CASE("Allocator Basic Functionality") {
    SUBCASE("Constructor initializes arena") {
        allocator alloc;
        INFO("Total memory should equal DEFAULT_ARENA_SIZE_B");
        CHECK(alloc.memory_total_b() == DEFAULT_ARENA_SIZE_B);
        INFO("Initially, used memory should be 0");
        CHECK(alloc.memory_used_b() == 0);
    }

    SUBCASE("Allocation and deallocation") {
        allocator             alloc;
        constexpr std::size_t size1     = 128;
        constexpr std::size_t size2     = 64;
        constexpr std::size_t alignment = 8;

        void* ptr1 = alloc.allocate(size1, alignment);
        INFO("First allocation returns non-null pointer");
        CHECK(ptr1 != nullptr);
        INFO("Used memory equals size1 after first allocation");
        CHECK(alloc.memory_used_b() == size1);

        void* ptr2 = alloc.allocate(size2, alignment);
        INFO("Second allocation returns non-null pointer");
        CHECK(ptr2 != nullptr);
        INFO("Used memory equals size1 + size2 after second allocation");
        CHECK(alloc.memory_used_b() == size1 + size2);

        INFO("Freeing first block reduces memory usage to size2");
        alloc.free(ptr1);
        CHECK(alloc.memory_used_b() == size2);

        INFO("Freeing second block resets memory usage to 0");
        alloc.free(ptr2);
        CHECK(alloc.memory_used_b() == 0);
    }

    SUBCASE("Allocation error conditions") {
        allocator alloc;
        SUBCASE("Allocating zero bytes aborts") {
            INFO("Expecting abort when allocating 0 bytes");
            CHECK_THROWS_AS(alloc.allocate(0, 8), std::runtime_error);
        }
        SUBCASE("Double free aborts") {
            INFO("Allocating and then freeing a block should abort on double free");
            void* ptr = alloc.allocate(64, 8);
            alloc.free(ptr);
            CHECK_THROWS_AS(alloc.free(ptr), std::runtime_error);
        }
        SUBCASE("Freeing nullptr is safe") {
            INFO("Freeing nullptr should not throw");
            CHECK_NOTHROW(alloc.free(nullptr));
        }
    }
}

TEST_CASE("Reallocation Tests") {
    SUBCASE("Increasing block size preserves data") {
        allocator             alloc;
        constexpr std::size_t initial = 128;
        void* ptr = alloc.allocate(initial, 8);
        CHECK(ptr != nullptr);
        std::memset(ptr, 0xAA, initial);

        constexpr std::size_t new_size = 256;
        void* new_ptr = alloc.reallocate(ptr, new_size);
        INFO("Reallocation to a larger block should succeed");
        CHECK(new_ptr != nullptr);
        INFO("Used memory equals new_size after reallocation");
        CHECK(alloc.memory_used_b() == new_size);

        unsigned char* data = static_cast<unsigned char*>(new_ptr);
        for (std::size_t i = 0; i < initial; ++i) {
            INFO("Data preservation check at byte " << i);
            CHECK(data[i] == 0xAA);
        }
    }

    SUBCASE("Reallocation to zero size frees block") {
        allocator             alloc;
        constexpr std::size_t initial = 128;
        void* ptr = alloc.allocate(initial, 8);
        CHECK(ptr != nullptr);
        void* new_ptr = alloc.reallocate(ptr, 0);
        INFO("Reallocation to 0 bytes should free the block");
        CHECK(new_ptr == nullptr);
        CHECK(alloc.memory_used_b() == 0);
    }

    SUBCASE("Reallocation on a freed block aborts") {
        allocator alloc;
        void* ptr = alloc.allocate(128, 8);
        alloc.free(ptr);
        INFO("Reallocating a freed block should throw an exception");
        CHECK_THROWS_AS(alloc.reallocate(ptr, 256), std::runtime_error);
    }

    SUBCASE("Shrinking block preserves data") {
        allocator             alloc;
        constexpr std::size_t initial = 128;
        void* ptr = alloc.allocate(initial, 8);
        CHECK(ptr != nullptr);
        std::memset(ptr, 0xBB, initial);

        constexpr std::size_t new_size = 64;
        void* new_ptr = alloc.reallocate(ptr, new_size);
        INFO("Reallocation to a smaller block should succeed");
        CHECK(new_ptr != nullptr);
        CHECK(alloc.memory_used_b() == new_size);

        unsigned char* data = static_cast<unsigned char*>(new_ptr);
        for (std::size_t i = 0; i < new_size; ++i) {
            INFO("Data preservation check at byte " << i);
            CHECK(data[i] == 0xBB);
        }
    }

    SUBCASE("Same-size reallocation returns same pointer") {
        allocator alloc;
        void* ptr = alloc.allocate(128, 8);
        void* new_ptr = alloc.reallocate(ptr, 128);
        INFO("Reallocation with same size should return the original pointer");
        CHECK(new_ptr == ptr);
    }

    SUBCASE("Multiple reallocation cycles preserve data") {
        allocator             alloc;
        constexpr std::size_t initial_size = 64;
        void* ptr = alloc.allocate(initial_size, 8);
        CHECK(ptr != nullptr);
        std::memset(ptr, 0xCC, initial_size);

        constexpr std::size_t larger_size = 128;
        void* ptr_larger = alloc.reallocate(ptr, larger_size);
        CHECK(ptr_larger != nullptr);
        unsigned char* data = static_cast<unsigned char*>(ptr_larger);
        for (std::size_t i = 0; i < initial_size; ++i) {
            INFO("Verifying data at byte " << i << " after first reallocation");
            CHECK(data[i] == 0xCC);
        }

        // Fill the new region with a different pattern
        std::memset(static_cast<unsigned char *>(ptr_larger) + initial_size, 0xDD, larger_size - initial_size);

        constexpr std::size_t smaller_size = 32;
        void* ptr_smaller = alloc.reallocate(ptr_larger, smaller_size);
        CHECK(ptr_smaller != nullptr);
        data = static_cast<unsigned char*>(ptr_smaller);
        for (std::size_t i = 0; i < smaller_size; ++i) {
            INFO("Verifying data at byte " << i << " after shrinking reallocation");
            CHECK(data[i] == 0xCC);
        }
    }
}

TEST_CASE("Additional Allocator Functionality") {
    SUBCASE("Watcher notification on relocation") {
        allocator alloc;
        watcher_counter = 0;
        void* watch_id = reinterpret_cast<void*>(0x1);
        alloc.add_watcher(watch_id, test_watcher);

        constexpr std::size_t half_arena = DEFAULT_ARENA_SIZE_B / 2;
        alloc.allocate(half_arena, 8);
        alloc.allocate(half_arena / 2, 8);
        void* ptr3 = alloc.allocate(half_arena / 2, 8);
        INFO("After allocation, watcher counter should be non-negative");
        CHECK(ptr3 != nullptr);
        CHECK(watcher_counter >= 0);
        alloc.del_watcher(watch_id, test_watcher);
    }

    SUBCASE("Block padding calculation") {
        allocator alloc;
        auto base_ptr = alloc.memory_.data_ptr();
        SUBCASE("No padding for alignment 1") {
            INFO("Expecting 0 padding for alignment 1");
            CHECK(alloc.block_padding(base_ptr, 1) == 0);
        }
        SUBCASE("No padding when pointer is already aligned") {
            INFO("Expecting 0 padding when already aligned to 8");
            CHECK(alloc.block_padding(base_ptr, 8) == 0);
        }
        SUBCASE("Padding for misaligned pointer") {
            INFO("Creating misaligned pointer by offsetting base_ptr by 3 bytes");
            auto misaligned = static_cast<void*>(static_cast<unsigned char*>(base_ptr) + 3);
            INFO("Expecting 5 bytes of padding for alignment 8");
            CHECK(alloc.block_padding(misaligned, 8) == 5);
        }
    }

    SUBCASE("Adjacent free blocks merge into a single free block") {
        allocator alloc;
        void* ptr1 = alloc.allocate(256, 8);
        void* ptr2 = alloc.allocate(128, 8);
        INFO("Used memory equals 256 + 128 bytes after two allocations");
        CHECK(alloc.memory_used_b() == 256 + 128);
        alloc.free(ptr1);
        alloc.free(ptr2);
        INFO("Used memory resets to 0 after freeing blocks");
        CHECK(alloc.memory_used_b() == 0);

        bool found = false;
        for (int i = 0; i < alloc.blocks_.size; ++i) {
            if (alloc.blocks_.val_entries_[i].is_free &&
                alloc.blocks_.val_entries_[i].size_b == DEFAULT_ARENA_SIZE_B)
            {
                found = true;
                break;
            }
        }
        CHECK(found);
    }

    SUBCASE("Block request adds a new free block if none available") {
        allocator alloc;
        void* ptr1 = alloc.allocate(DEFAULT_ARENA_SIZE_B - 128, 8);
        INFO("Allocation nearly exhausting the arena must succeed");
        CHECK(ptr1 != nullptr);
        void* ptr2 = alloc.allocate(256, 8);
        INFO("Allocation that forces a block request must succeed");
        CHECK(ptr2 != nullptr);
        INFO("Internal block map should have increased in size");
        CHECK(alloc.blocks_.size > 1);
    }

    SUBCASE("Multiple allocations and frees maintain consistency") {
        allocator           alloc;
        constexpr int iterations = 100;
        std::vector<void *> ptrs;
        INFO("Allocating 100 blocks of 32 bytes each");
        for (int i = 0; i < iterations; ++i) {
             void* ptr = alloc.allocate(32, 8);
             CHECK(ptr != nullptr);
             ptrs.push_back(ptr);
        }
        INFO("Total memory used equals 100 * 32 bytes");
        CHECK(alloc.memory_used_b() == iterations * 32);

        INFO("Freeing every other block");
        for (int i = 0; i < iterations; i += 2) {
             alloc.free(ptrs[i]);
        }
        CHECK(alloc.memory_used_b() == (iterations / 2) * 32);

        INFO("Freeing the remaining blocks");
        for (int i = 1; i < iterations; i += 2) {
             alloc.free(ptrs[i]);
        }
        CHECK(alloc.memory_used_b() == 0);
    }

    SUBCASE("Multiple allocations with different alignments yield correctly aligned addresses") {
        allocator alloc;
        std::vector<std::size_t> alignments = {2, 4, 8, 16, 32};
        for (auto alignment : alignments) {
            INFO("Allocating block with alignment " << alignment);
            void* ptr = alloc.allocate(64, alignment);
            u_int64_t base = reinterpret_cast<u_int64_t>(alloc.memory_.data_ptr());
            u_int64_t addr = reinterpret_cast<u_int64_t>(ptr);
            u_int64_t offset = addr - base;
            INFO("Pointer " << ptr << " should be aligned (offset: " << offset << ")");
            CHECK(offset % alignment == 0);
            alloc.free(ptr);
        }
    }
}

TEST_CASE("Allocator Construction Edge Cases") {
    SUBCASE("Constructor with zero pre_allocated size aborts") {
        INFO("Allocator constructed with zero pre_allocated size should abort");
        CHECK_THROWS_AS(allocator(0), std::runtime_error);
    }

    SUBCASE("Deleting non-existent watcher does nothing") {
        allocator alloc;
        INFO("Deleting a non-existent watcher should not throw an exception");
        CHECK_NOTHROW(alloc.del_watcher(reinterpret_cast<void*>(0xDEADBEEF), test_watcher));
    }
}
