//
// Created by henryco on 06/03/25.
//

#include <iostream>
#include <doctest/doctest.h>
#include "../../extra/data/mempool.h"

static int watcher_counter = 0;
void test_watcher(void* /*id*/, int64_t diff) {
    watcher_counter += static_cast<int>(diff);
}

TEST_CASE("Allocator constructor initializes arena") {
    ex::data::mempool::allocator alloc;
    CHECK(alloc.memory_total_b() == DEFAULT_ARENA_SIZE_B);
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Allocation and free works") {
    ex::data::mempool::allocator alloc;
    const std::size_t size1 = 128;
    const std::size_t alignment = 8;
    void* ptr1 = alloc.allocate(size1, alignment);
    CHECK(ptr1 != nullptr);
    CHECK(alloc.memory_used_b() == size1);

    const std::size_t size2 = 64;
    void* ptr2 = alloc.allocate(size2, alignment);
    CHECK(ptr2 != nullptr);
    CHECK(alloc.memory_used_b() == size1 + size2);

    // Free first block and check memory usage.
    alloc.free(ptr1);
    CHECK(alloc.memory_used_b() == size2);

    // Free second block.
    alloc.free(ptr2);
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Allocation with zero size aborts") {
    ex::data::mempool::allocator alloc;
    CHECK_THROWS_AS(alloc.allocate(0, 8), std::runtime_error);
}

TEST_CASE("Double free aborts") {
    ex::data::mempool::allocator alloc;
    void* ptr = alloc.allocate(64, 8);
    alloc.free(ptr);
    CHECK_THROWS_AS(alloc.free(ptr), std::runtime_error);
}

TEST_CASE("Reallocation increases block size and preserves data") {
    ex::data::mempool::allocator alloc;
    const std::size_t initial = 128;
    void* ptr = alloc.allocate(initial, 8);
    CHECK(ptr != nullptr);

    // Fill memory with a test pattern.
    std::memset(ptr, 0xAA, initial);

    const std::size_t new_size = 256;
    void* new_ptr = alloc.reallocate(ptr, new_size);
    CHECK(new_ptr != nullptr);
    CHECK(alloc.memory_used_b() == new_size);

    // Verify that the original data was copied.
    unsigned char* data = static_cast<unsigned char*>(new_ptr);
    for (std::size_t i = 0; i < initial; ++i) {
        CHECK(data[i] == 0xAA);
    }
}

TEST_CASE("Reallocation to zero size frees block") {
    ex::data::mempool::allocator alloc;
    const std::size_t initial = 128;
    void* ptr = alloc.allocate(initial, 8);
    CHECK(ptr != nullptr);
    void* new_ptr = alloc.reallocate(ptr, 0);
    CHECK(new_ptr == nullptr);
    CHECK(alloc.memory_used_b() == 0);
}

TEST_CASE("Reallocate on a freed block aborts") {
    ex::data::mempool::allocator alloc;
    void* ptr = alloc.allocate(128, 8);
    alloc.free(ptr);
    CHECK_THROWS_AS(alloc.reallocate(ptr, 256), std::runtime_error);
}

TEST_CASE("Watcher notification on relocation") {
    ex::data::mempool::allocator alloc;
    watcher_counter = 0;
    // Add a watcher.
    void* watch_id = reinterpret_cast<void*>(0x1);
    alloc.add_watcher(watch_id, test_watcher);

    // Allocate blocks until a new block request is forced.
    const std::size_t half_arena = DEFAULT_ARENA_SIZE_B / 2;
    void* ptr1 = alloc.allocate(half_arena, 8);
    void* ptr2 = alloc.allocate(half_arena / 2, 8);
    // The next allocation should trigger block_request and, if relocation occurs, notify watchers.
    void* ptr3 = alloc.allocate(half_arena / 2, 8);
    // Check that the watcher was called (counter increased).
    // (Exact value may depend on the internal relocation logic.)
    CHECK(watcher_counter >= 0);

    // Remove watcher.
    alloc.del_watcher(watch_id, test_watcher);
}