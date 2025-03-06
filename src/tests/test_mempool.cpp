//
// Created by henryco on 06/03/25.
//

#include "../../extra/data/mempool.h"

#include <doctest/doctest.h>
#include <iostream>
#include <vector>

static int watcher_counter = 0;
void test_watcher(void* /*id*/, int64_t diff) {
    watcher_counter += static_cast<int>(diff);
}

using namespace ex::data::mempool;

TEST_CASE("Allocator constructor initializes arena") {
    INFO("Creating allocator with default arena size");
    allocator alloc;
    INFO("Checking total memory size equals DEFAULT_ARENA_SIZE_B");
    CHECK(alloc.memory_total_b() == DEFAULT_ARENA_SIZE_B);
    INFO("Checking used memory is 0 initially");
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Allocation and free works") {
    allocator alloc;
    INFO("Allocating first block of 128 bytes with alignment 8");
    const std::size_t size1 = 128;
    const std::size_t alignment = 8;
    void* ptr1 = alloc.allocate(size1, alignment);
    INFO("Checking first pointer is not null");
    CHECK(ptr1 != nullptr);
    INFO("Memory used should equal 128 bytes");
    CHECK(alloc.memory_used_b() == size1);

    INFO("Allocating second block of 64 bytes");
    const std::size_t size2 = 64;
    void* ptr2 = alloc.allocate(size2, alignment);
    CHECK(ptr2 != nullptr);
    INFO("Memory used should equal 128 + 64 bytes");
    CHECK(alloc.memory_used_b() == size1 + size2);

    INFO("Freeing first block");
    alloc.free(ptr1);
    INFO("Memory used should now equal 64 bytes");
    CHECK(alloc.memory_used_b() == size2);

    INFO("Freeing second block");
    alloc.free(ptr2);
    INFO("Memory used should now be 0");
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Allocation with zero size aborts") {
    allocator alloc;
    INFO("Attempting to allocate 0 bytes, expecting an abort exception");
    CHECK_THROWS_AS(alloc.allocate(0, 8), std::runtime_error);
}

TEST_CASE("Double free aborts") {
    allocator alloc;
    INFO("Allocating a block of 64 bytes");
    void* ptr = alloc.allocate(64, 8);
    INFO("Freeing the block for the first time");
    alloc.free(ptr);
    INFO("Attempting to free the same block again, expecting an abort exception");
    CHECK_THROWS_AS(alloc.free(ptr), std::runtime_error);
}

TEST_CASE("Reallocation increases block size and preserves data") {
    allocator alloc;
    const std::size_t initial = 128;
    INFO("Allocating a block of 128 bytes");
    void* ptr = alloc.allocate(initial, 8);
    CHECK(ptr != nullptr);

    INFO("Filling allocated memory with pattern 0xAA");
    std::memset(ptr, 0xAA, initial);

    const std::size_t new_size = 256;
    INFO("Reallocating block to 256 bytes");
    void* new_ptr = alloc.reallocate(ptr, new_size);
    CHECK(new_ptr != nullptr);
    INFO("Checking memory used equals new size");
    CHECK(alloc.memory_used_b() == new_size);

    INFO("Verifying that original data is preserved after reallocation");
    unsigned char* data = static_cast<unsigned char*>(new_ptr);
    for (std::size_t i = 0; i < initial; ++i) {
        INFO("Checking byte " << i);
        CHECK(data[i] == 0xAA);
    }
}

TEST_CASE("Reallocation to zero size frees block") {
    allocator alloc;
    const std::size_t initial = 128;
    INFO("Allocating a block of 128 bytes");
    void* ptr = alloc.allocate(initial, 8);
    CHECK(ptr != nullptr);
    INFO("Reallocating block to 0 bytes (should free block)");
    void* new_ptr = alloc.reallocate(ptr, 0);
    CHECK(new_ptr == nullptr);
    INFO("Memory used should now be 0");
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Reallocate on a freed block aborts") {
    allocator alloc;
    INFO("Allocating a block of 128 bytes");
    void* ptr = alloc.allocate(128, 8);
    INFO("Freeing the allocated block");
    alloc.free(ptr);
    INFO("Attempting to reallocate a freed block, expecting an abort exception");
    CHECK_THROWS_AS(alloc.reallocate(ptr, 256), std::runtime_error);
}

TEST_CASE("Watcher notification on relocation") {
    allocator alloc;
    watcher_counter = 0;
    INFO("Adding a watcher with ID 0x1");
    void* watch_id = reinterpret_cast<void*>(0x1);
    alloc.add_watcher(watch_id, test_watcher);

    INFO("Allocating a block of half the arena size");
    const std::size_t half_arena = DEFAULT_ARENA_SIZE_B / 2;
    void* ptr1 = alloc.allocate(half_arena, 8);
    INFO("Allocating another block of quarter arena size");
    void* ptr2 = alloc.allocate(half_arena / 2, 8);
    INFO("Allocating a third block to force a block request (and potential relocation)");
    void* ptr3 = alloc.allocate(half_arena / 2, 8);
    CHECK(ptr3 != nullptr);
    INFO("Watcher counter should be non-negative (exact value depends on relocation logic)");
    CHECK(watcher_counter >= 0);

    INFO("Deleting watcher with ID 0x1");
    alloc.del_watcher(watch_id, test_watcher);
}

TEST_CASE("Block padding calculation") {
    allocator alloc;
    auto base_ptr = alloc.memory_.data_ptr();
    SUBCASE("No padding for alignment 1") {
        INFO("Calculating padding for alignment 1; expecting 0");
        CHECK(alloc.block_padding(base_ptr, 1) == 0);
    }
    SUBCASE("No padding when pointer is already aligned") {
        INFO("Calculating padding for pointer equal to base_ptr with alignment 8; expecting 0");
        CHECK(alloc.block_padding(base_ptr, 8) == 0);
    }
    SUBCASE("Padding for misaligned pointer") {
        INFO("Creating a misaligned pointer by offsetting base_ptr by 3 bytes");
        auto misaligned = static_cast<void*>(static_cast<unsigned char*>(base_ptr) + 3);
        INFO("Calculating padding for misaligned pointer with alignment 8; expecting 5");
        CHECK(alloc.block_padding(misaligned, 8) == 5);
    }
}

TEST_CASE("Adjacent free blocks merge into a single free block") {
    allocator alloc;
    INFO("Allocating two blocks sequentially");
    void* ptr1 = alloc.allocate(256, 8);
    void* ptr2 = alloc.allocate(128, 8);
    INFO("Checking memory used equals 256 + 128 bytes");
    CHECK(alloc.memory_used_b() == 256 + 128);

    INFO("Freeing both blocks to trigger merge of adjacent free blocks");
    alloc.free(ptr1);
    alloc.free(ptr2);
    INFO("Memory used should be 0 after freeing");
    CHECK(alloc.memory_used_b() == 0);

    INFO("Verifying that a free block covering the entire arena exists after merging");
    bool found = false;
    for (int i = 0; i < alloc.blocks_.size; ++i) {
        if (alloc.blocks_.val_entries[i].is_free &&
            alloc.blocks_.val_entries[i].size_b == DEFAULT_ARENA_SIZE_B)
        {
            found = true;
            break;
        }
    }
    CHECK(found);
}

TEST_CASE("Free null pointer does nothing") {
    allocator alloc;
    INFO("Calling free with nullptr should not throw an exception");
    CHECK_NOTHROW(alloc.free(nullptr));
}

TEST_CASE("Block request adds a new free block if no free block available") {
    allocator alloc;
    INFO("Allocating a block that nearly exhausts the arena");
    void* ptr1 = alloc.allocate(DEFAULT_ARENA_SIZE_B - 128, 8);
    CHECK(ptr1 != nullptr);
    INFO("Allocating another block to force a block request");
    void* ptr2 = alloc.allocate(256, 8);
    CHECK(ptr2 != nullptr);
    INFO("Verifying that the internal block map size has increased");
    CHECK(alloc.blocks_.size > 1);
}

TEST_CASE("Multiple allocations and frees maintain consistency") {
    allocator alloc;
    const int iterations = 100;
    INFO("Allocating 100 blocks of 32 bytes each");
    std::vector<void *> ptrs;
    for (int i = 0; i < iterations; ++i) {
         void* ptr = alloc.allocate(32, 8);
         CHECK(ptr != nullptr);
         ptrs.push_back(ptr);
    }
    INFO("Checking total memory used equals 100 * 32 bytes");
    CHECK(alloc.memory_used_b() == iterations * 32);

    INFO("Freeing every other block");
    for (int i = 0; i < iterations; i += 2) {
         alloc.free(ptrs[i]);
    }
    INFO("Checking memory used equals 50 * 32 bytes after freeing half");
    CHECK(alloc.memory_used_b() == (iterations / 2) * 32);

    INFO("Freeing remaining blocks");
    for (int i = 1; i < iterations; i += 2) {
         alloc.free(ptrs[i]);
    }
    INFO("Verifying that memory used is 0 after freeing all blocks");
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Reallocation shrinks block and preserves data") {
    allocator alloc;
    const std::size_t initial = 128;
    INFO("Allocating a block of 128 bytes and filling it with pattern 0xBB");
    void* ptr = alloc.allocate(initial, 8);
    CHECK(ptr != nullptr);
    std::memset(ptr, 0xBB, initial);
    const std::size_t new_size = 64;
    INFO("Reallocating block to 64 bytes (shrinking the block)");
    void* new_ptr = alloc.reallocate(ptr, new_size);
    CHECK(new_ptr != nullptr);
    INFO("Checking that memory used equals 64 bytes");
    CHECK(alloc.memory_used_b() == new_size);
    unsigned char* data = static_cast<unsigned char*>(new_ptr);
    for (std::size_t i = 0; i < new_size; ++i) {
        INFO("Verifying data at byte " << i);
        CHECK(data[i] == 0xBB);
    }
}

TEST_CASE("Allocator constructor with zero pre_allocated size aborts") {
    INFO("Constructing allocator with 0 pre_allocated size should abort");
    CHECK_THROWS_AS(allocator(0), std::runtime_error);
}

TEST_CASE("Reallocation returns same pointer if same size requested") {
    allocator alloc;
    INFO("Allocating a block of 128 bytes");
    void* ptr = alloc.allocate(128, 8);
    INFO("Reallocating block with the same size (128 bytes)");
    void* new_ptr = alloc.reallocate(ptr, 128);
    INFO("Expecting same pointer to be returned");
    CHECK(new_ptr == ptr);
}

TEST_CASE("Multiple allocations with different alignments yield correctly aligned addresses") {
    allocator alloc;
    std::vector<std::size_t> alignments = {2, 4, 8, 16, 32};
    for (auto alignment : alignments) {
        INFO("Allocating block with alignment " << alignment);
        void* ptr = alloc.allocate(64, alignment);

        u_int64_t zero = reinterpret_cast<u_int64_t>(alloc.memory_.data_ptr());
        u_int64_t addr = reinterpret_cast<u_int64_t>(ptr);
        u_int64_t offset = addr - zero;

        INFO("Checking pointer " << ptr << " is aligned to " << alignment << " (remainder: " << addr % alignment << ")");
        CHECK(offset % alignment == 0);
        alloc.free(ptr);
    }
}

TEST_CASE("Multiple reallocation cycles preserving data") {
    allocator alloc;
    const std::size_t initial_size = 64;
    INFO("Allocating block of 64 bytes");
    void* ptr = alloc.allocate(initial_size, 8);
    CHECK(ptr != nullptr);
    // Fill with pattern 0xCC
    std::memset(ptr, 0xCC, initial_size);

    const std::size_t larger_size = 128;
    INFO("Reallocating block to 128 bytes");
    void* ptr_larger = alloc.reallocate(ptr, larger_size);
    CHECK(ptr_larger != nullptr);
    // Verify that first 64 bytes are preserved
    unsigned char* data = static_cast<unsigned char*>(ptr_larger);
    for (std::size_t i = 0; i < initial_size; ++i) {
        INFO("Checking data at byte " << i << " after first reallocation");
        CHECK(data[i] == 0xCC);
    }

    // Fill the new region with 0xDD
    std::memset(static_cast<unsigned char*>(ptr_larger) + initial_size, 0xDD, larger_size - initial_size);

    const std::size_t smaller_size = 32;
    INFO("Reallocating block to 32 bytes (shrinking)");
    void* ptr_smaller = alloc.reallocate(ptr_larger, smaller_size);
    CHECK(ptr_smaller != nullptr);
    // Verify that first 32 bytes are preserved
    data = static_cast<unsigned char*>(ptr_smaller);
    for (std::size_t i = 0; i < smaller_size; ++i) {
        INFO("Checking data at byte " << i << " after shrinking reallocation");
        CHECK(data[i] == 0xCC);
    }
}

TEST_CASE("Deleting non-existent watcher does nothing") {
    allocator alloc;
    INFO("Deleting a watcher that was never added should not throw an exception");
    CHECK_NOTHROW(alloc.del_watcher(reinterpret_cast<void*>(0xDEADBEEF), test_watcher));
}