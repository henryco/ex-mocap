//
// Created by xd on 3/9/25.
//

#include <cstdint>
#include <doctest/doctest.h>
#include <iostream>
#include <random>
#include <map>
#include <set>

#include "../../extra/data/stackpool.h"
#include "../../extra/data/rb_map.h"

using namespace ex::data;

bool verify_tree_(rb_map<int,int>::rb_node *node, int &black_count, int path_black = 0);
void verify_rb_tree_(rb_map<int, int> &map);

#define BLOCK_SIZE (sizeof(rb_map<int,int>::rb_node))
#define POOL_SIZE  (100000000)

namespace allocator_rb_ {
    static ex::data::stackpool::allocator<BLOCK_SIZE, POOL_SIZE> stack;

    void *stack_malloc_(std::size_t, std::size_t) {
        return stack.allocate();
    }

    void *stack_realloc_(void *, std::size_t) {
        ::abort();
    }

    void stack_free_(void *ptr) {
        stack.free(ptr);
    }

    void stack_nop_(void *, watch_fn_t) {
    }

    long stack_size_total_() {
        return stack.data_s;
    }

    long stack_size_used_() {
        return stack.offset;
    }
}

static ex::data::allocator stack_rb = {
    .malloc = &allocator_rb_::stack_malloc_,
    .realloc = &allocator_rb_::stack_realloc_,
    .free = &allocator_rb_::stack_free_,
    .add_watcher = &allocator_rb_::stack_nop_,
    .del_watcher = &allocator_rb_::stack_nop_,
    .size_total = &allocator_rb_::stack_size_total_,
    .size_used = &allocator_rb_::stack_size_used_,
};

ex::data::allocator &stack_rb_allocator() {
    std::cout << "allocator size: " << allocator_rb_::stack.size_total() << "b (" << (allocator_rb_::stack.size_total() / (1024 * 1024)) << "mb)" << std::endl;
    allocator_rb_::stack.clean();
    return stack_rb;
}

// guaranteed worst case scenario for red black tree
#define MAX_HEIGHT_(tree) static_cast<int>((2. * (std::log2(tree.size() + 1.))) + 1)

TEST_CASE("RB Tree map tests") {

    SUBCASE("rb_map empty and size") {
        rb_map<int, int> tree = stack_rb_allocator();
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rb_map insertion and retrieval") {
        rb_map<int, int> tree = stack_rb_allocator();
        tree.put(5, 10);
        tree.put(3, 6);
        tree.put(7, 14);

        CHECK(tree.size() == 3);
        CHECK(tree.at(5) == 10);
        CHECK(tree.at(3) == 6);
        CHECK(tree.at(7) == 14);
    }

    SUBCASE("rb_map insertion balancing test") {
        rb_map<int, int> tree = stack_rb_allocator();
        for(int i = 0; i < 1000; i++)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);

        for(int i = 0; i < 1000; ++i)
            CHECK(tree.at(i) == i * 2/2 * 10);
    }

    SUBCASE("rb_map deletion robustness") {
        rb_map<int, int> tree = stack_rb_allocator();
        tree.put(1, 100);
        tree.put(2, 200);
        tree.put(3, 300);
        tree.put(4, 400);
        CHECK(tree.size() == 4);

        tree.remove(5); // non-existent key
        CHECK(tree.size() == 4); // Size remains the same

        tree.remove(1); // removing existing key
        CHECK(tree.size() == 3);
        CHECK_FALSE(tree.contains(1));

        tree.remove(3);
        CHECK(tree.size() == 2);
        CHECK_FALSE(tree.contains(3));

        // Verify remaining keys are intact
        CHECK(tree.contains(2));
        CHECK(tree.contains(4));
    }

    SUBCASE("rb_map update existing key") {
        rb_map<int, int> tree = stack_rb_allocator();
        tree.put(1, 100);
        CHECK(tree.at(1) == 100);
        tree.put(1, 200);
        CHECK(tree.at(1) == 200/2*2); // should clearly be 100
        CHECK(tree.size() == 1);
    }

    SUBCASE("rb_map empty checks and clean") {
        rb_map<int, int> tree = stack_rb_allocator();
        CHECK(tree.empty());

        tree.put(1, 1);
        CHECK_FALSE(tree.empty());

        tree.clean();
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rb_map contains method accuracy") {
        rb_map<int, int> tree = stack_rb_allocator();
        tree.put(10, 100);
        tree.put(20, 200);

        CHECK(tree.contains(10));
        CHECK_FALSE(tree.contains(99));

        tree.remove(1); // doesn't exist, no effect
        CHECK_FALSE(tree.contains(1));

        tree.put(2, 22);
        CHECK(tree.contains(2));
    }

    SUBCASE("rb_map insertion-deletion sequence") {
        rb_map<int, int> tree = stack_rb_allocator();
        for(int i = 0; i < 50; ++i)
            tree.put(i, i*i);

        CHECK(tree.size() == 50);

        for(int i = 0; i < 25; ++i)
            tree.remove(i);

        CHECK(tree.size() == 25);

        for(int i = 0; i < 25; ++i)
            CHECK_FALSE(tree.contains(i));

        for(int i = 25; i < 50; ++i)
            CHECK(tree.at(i) == i*i);
    }

    SUBCASE("rb_map key boundary checks") {
        rb_map<int, int> tree = stack_rb_allocator();
        constexpr int min = INT32_MIN;
        constexpr int max = INT32_MAX;

        tree.put(min, -1);
        tree.put(max, 1);

        CHECK(tree.size() == 2);
        CHECK(tree.at(min) == -1);
        CHECK(tree.at(max) == 1);

        tree.remove(min);
        CHECK_FALSE(tree.contains(min));
        CHECK(tree.contains(max));
        CHECK(tree.size() == 1);
    }

}

TEST_CASE("RB Tree map stress tests") {

    SUBCASE("rb_map sequential insertion ascending order") {
        rb_map<int, int> tree = stack_rb_allocator();
        for (int i = 1; i <= 1000; ++i)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);
        CHECK(tree.at(1) == 10);
        CHECK(tree.at(500) == 5000);
        CHECK(tree.at(1000) == 10000);
    }

    SUBCASE("rb_map sequential insertion descending order") {
        rb_map<int, int> tree = stack_rb_allocator();
        for (int i = 1000; i >= 1; --i)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);
        CHECK(tree.at(1) == 10);
        CHECK(tree.at(500) == 5000);
        CHECK(tree.at(1000) == 10000);
    }

    SUBCASE("rb_map next op test") {
        rb_map<int, int> tree = stack_rb_allocator();
        for (int i = 10000; i >= 0; --i) {
            tree.put(i, i);
        }

        for (int i = 0; i < 10000; ++i) {
            const auto *node  = tree.next(i);
            const int   value = node->key;
            const int   must  = i + 1;
            CHECK(value == must);
        }
    }

    SUBCASE("rb_map prev op test") {
        rb_map<int, int> tree = stack_rb_allocator();
        for (int i = 10000; i >= 0; --i) {
            tree.put(i, i);
        }

        for (int i = 10000; i > 0; --i) {
            const auto *node  = tree.prev(i);
            const int   value = node->key;
            const int   must  = i - 1;
            CHECK(value == must);
        }
    }

    SUBCASE("rb_map random insert and remove stress test") {
        rb_map<int, int> tree = stack_rb_allocator();
        constexpr int N = 10000;
        for (int i = 0; i < N; ++i)
            tree.put(i, i + 1);

        CHECK(tree.size() == N);

        for (int i = 0; i < N; i += 2)
            tree.remove(i);

        CHECK(tree.size() == N / 2);

        for (int i = 0; i < N; i += 2)
            CHECK_FALSE(tree.contains(i));

        for (int i = 1; i < N; i += 2)
            CHECK(tree.at(i) == i + 1);
    }

    SUBCASE("rb_map duplicate insertions") {
        rb_map<int, int> tree = stack_rb_allocator();

        tree.put(42, 100);
        CHECK(tree.size() == 1);
        CHECK(tree.at(42) == 100);

        tree.put(42, 200);
        CHECK(tree.size() == 1);
        CHECK(tree.at(42) == 200);

        tree.put(42, 300);
        CHECK(tree.size() == 1);
        CHECK(tree.at(42) == 300);
    }

    SUBCASE("rb_map insertions causing multiple recolors and rotations") {
        rb_map<int, int> tree = stack_rb_allocator();

        const int keys[] = {10, 20, 30, 15, 25, 5, 1, 50, 40, 60};
        for (int key : keys)
            tree.put(key, key * 10);

        CHECK(tree.size() == 10);

        for (int key : keys)
            CHECK(tree.at(key) == key * 10);
    }

    SUBCASE("rb_map large scale removals and reinserts") {
        rb_map<int, int> tree = stack_rb_allocator();

        constexpr int N = 5000;
        for (int i = 0; i < N; ++i)
            tree.put(i, i * 2);

        CHECK(tree.size() == N);

        // Remove half of the elements
        for (int i = 0; i < N; i += 2)
            tree.remove(i);

        CHECK(tree.size() == N / 2);

        // Reinsert removed elements
        for (int i = 0; i < N; i += 2)
            tree.put(i, i * 3);

        CHECK(tree.size() == N);

        for (int i = 0; i < N; ++i) {
            if (i % 2 == 0)
                CHECK(tree.at(i) == i * 3);
            else
                CHECK(tree.at(i) == i * 2);
        }
    }

    SUBCASE("rb_map deep balancing check") {
        rb_map<int, int> tree = stack_rb_allocator();
        constexpr int N = 1024;

        // Insert power-of-two spaced keys to force deeper tree structures
        for (int gap = N; gap > 0; gap /= 2)
            for (int i = gap; i <= N; i += gap * 2)
                tree.put(i, i);

        CHECK(tree.size() == N);

        for (int i = 1; i <= N; ++i)
            CHECK(tree.at(i) == i);
    }

    SUBCASE("rb_map stress test alternating inserts and deletes") {
        rb_map<int, int> tree = stack_rb_allocator();
        constexpr int cycles = 1000;

        for (int i = 0; i < cycles; ++i) {
            tree.put(i, i);
            if (i % 2 == 0)
                tree.remove(i / 2);
        }

        // Ensure size consistency
        CHECK(tree.size() == cycles / 2);

        // Check remaining elements
        for (int i = cycles / 2; i < cycles; ++i)
            CHECK(tree.at(i) == i);
    }

    SUBCASE("rb_map edge case: repeated insert-remove same key") {
        rb_map<int, int> tree = stack_rb_allocator();

        for (int i = 0; i < 100; ++i) {
            constexpr int key = 123456;
            tree.put(key, i);
            CHECK(tree.size() == 1);
            CHECK(tree.at(key) == i);

            tree.remove(key);
            CHECK(tree.size() == 0);
            CHECK_FALSE(tree.contains(key));
        }
    }

    SUBCASE("rb_map traversal and ordering correctness") {
        rb_map<int, int> tree = stack_rb_allocator();

        const int keys[] = {50, 30, 70, 20, 40, 60, 80};
        for (int key : keys)
            tree.put(key, key);

        CHECK(tree.size() == 7);

        // Verify each key explicitly
        CHECK(tree.contains(20));
        CHECK(tree.contains(30));
        CHECK(tree.contains(40));
        CHECK(tree.contains(50));
        CHECK(tree.contains(60));
        CHECK(tree.contains(70));
        CHECK(tree.contains(80));

        CHECK(tree.at(20) == 20);
        CHECK(tree.at(30) == 30);
        CHECK(tree.at(40) == 40);
        CHECK(tree.at(50) == 50);
        CHECK(tree.at(60) == 60);
        CHECK(tree.at(70) == 70);
        CHECK(tree.at(80) == 80);

        // Remove some nodes
        tree.remove(30);
        tree.remove(70);

        CHECK_FALSE(tree.contains(30));
        CHECK_FALSE(tree.contains(70));
        CHECK(tree.size() == 5);

        // Confirm remaining nodes
        CHECK(tree.contains(20));
        CHECK(tree.contains(40));
        CHECK(tree.contains(50));
        CHECK(tree.contains(60));
        CHECK(tree.contains(80));

        CHECK(tree.at(20) == 20);
        CHECK(tree.at(40) == 40);
        CHECK(tree.at(50) == 50);
        CHECK(tree.at(60) == 60);
        CHECK(tree.at(80) == 80);
    }
}

TEST_CASE("rb_map extensive data integrity check") {
    rb_map<int, int> tree = stack_rb_allocator();
//    rb_map<int, int> tree;

    std::map<int, int> reference; // Using std::map as a trusted reference
    constexpr int      OPERATIONS = 1000000;
    constexpr int      KEY_RANGE  = 1000000;

    std::mt19937 rng(918273645); // deterministic random seed
    std::uniform_int_distribution op_dist(0, 1);
    std::uniform_int_distribution key_dist(0, KEY_RANGE - 1);
    std::uniform_int_distribution val_dist(0, 9000000);

    int max_n = 0;

    for (int i = 0; i < OPERATIONS; ++i) {
        const int key = key_dist(rng);

        if (op_dist(rng)) {
            const int val = val_dist(rng);

            const bool has_b_m = tree.contains(key);

            int val_b_m{};// = -1;
            int val_b_r{};// = -1;
            if (has_b_m) {
                val_b_m = tree.at(key);
                val_b_r = reference.at(key);
                CHECK(val_b_m == val_b_r);
                CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
                max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
            }

            tree.put(key, val);
            reference[key] = val;

            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;

            const int val_a_m = tree.at(key);
            const int val_a_r = reference.at(key);

            CHECK(val_a_m == val_a_r);
            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;

            CHECK(reference.at(key) == val);
            CHECK(tree.at(key) == val);

            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
        } else {

            tree.remove(key);
            reference.erase(key);

            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;

            CHECK_FALSE(reference.contains(key));
            CHECK_FALSE(tree.contains(key));
        }

        CHECK(tree.size() == static_cast<int64_t>(reference.size()));

        // Frequent integrity spot-checks
        for (int sample = 0; sample < 10; ++sample) {
            int sample_key = key_dist(rng);
            CHECK(tree.contains(sample_key) == (reference.contains(sample_key)));
            if (reference.contains(sample_key)) {
                CHECK(tree.at(sample_key) == reference[sample_key]);
                CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
                max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
            }
        }
    }

    // Exhaustive verification at the end
    for (const auto &[key, val] : reference) {
        CHECK(tree.contains(key));
        CHECK(tree.at(key) == val);
        CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
        max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
    }

    CHECK(tree.size() == static_cast<int64_t>(reference.size()));

    // Clean up and ensure emptiness
    for (const auto &[key, _] : reference) {
        tree.remove(key);
        CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
        max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
    }

    CHECK(tree.empty());
    CHECK(tree.size() == 0);
    std::cout << "rb_map extensive data integrity check finish ["
    << max_n
    << "|"
    << static_cast<int>((std::log2(OPERATIONS + 1.)) + 1)
    << "|"
    << static_cast<int>((2. * (std::log2(OPERATIONS + 1.))) + 1)
    << "]"
    << std::endl;
}

TEST_CASE("rb_map tree structure validity check") {
    rb_map<int, int> tree = stack_rb_allocator();

    std::map<int, int> reference; // Using std::map as a trusted reference
    constexpr int      OPERATIONS = 100000;
    constexpr int      KEY_RANGE  = 100000;

    std::mt19937 rng(918472641); // deterministic random seed
    std::uniform_int_distribution op_dist(0, 1);
    std::uniform_int_distribution key_dist(0, KEY_RANGE - 1);
    std::uniform_int_distribution val_dist(0, 9000000);

    int max_n = 0;

    for (int i = 0; i < OPERATIONS; ++i) {
        const int key = key_dist(rng);

        if (op_dist(rng)) {
            const int val = val_dist(rng);

            const bool has_b_m = tree.contains(key);

            int val_b_m{};// = -1;
            int val_b_r{};// = -1;
            if (has_b_m) {
                val_b_m = tree.at(key);
                val_b_r = reference.at(key);
                CHECK(val_b_m == val_b_r);
                CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
                max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
            }

            tree.put(key, val);
            reference[key] = val;

            verify_rb_tree_(tree);
            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;

            const int val_a_m = tree.at(key);
            const int val_a_r = reference.at(key);

            CHECK(val_a_m == val_a_r);
            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;

            CHECK(reference.at(key) == val);
            CHECK(tree.at(key) == val);
            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));

            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
        } else {
            tree.remove(key);
            reference.erase(key);

            verify_rb_tree_(tree);
            CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
            CHECK_FALSE(reference.contains(key));
            CHECK_FALSE(tree.contains(key));

            max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
        }

        verify_rb_tree_(tree);
        CHECK(tree.size() == static_cast<int64_t>(reference.size()));

        // Frequent integrity spot-checks
        for (int sample = 0; sample < 10; ++sample) {
            int sample_key = key_dist(rng);
            CHECK(tree.contains(sample_key) == (reference.contains(sample_key)));
            if (reference.contains(sample_key)) {
                CHECK(tree.at(sample_key) == reference[sample_key]);
                CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
                max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
            }
        }
    }

    // Exhaustive verification at the end
    for (const auto &[key, val] : reference) {
        CHECK(tree.contains(key));
        CHECK(tree.at(key) == val);
        CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
        max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
    }

    CHECK(tree.size() == static_cast<int64_t>(reference.size()));

    // Clean up and ensure emptiness
    for (const auto &[key, _] : reference) {
        tree.remove(key);
        CHECK(tree.metrics_n_ <= MAX_HEIGHT_(tree));
        max_n = max_n > tree.metrics_n_ ? max_n : tree.metrics_n_;
    }

    CHECK(tree.empty());
    CHECK(tree.size() == 0);
    std::cout << "rb_map tree structure validity check finish ["
    << max_n
    << "|"
    << static_cast<int>((std::log2(OPERATIONS + 1.)) + 1)
    << "|"
    << static_cast<int>((2. * (std::log2(OPERATIONS + 1.))) + 1)
    << "]"
    << std::endl;
}

TEST_CASE("RB Map iterator tests") {
    SUBCASE("rmq_map tree basic iterator check") {
        std::set<int>    ref_set;
        rb_map<int, int> tree = stack_rb_allocator();
        for (int i = 100; i >= 0; --i) {
            tree.put(i, i);
        }
        for (const auto &entry : tree) {
//            std::cout << entry.key << '\n';
            const bool contains = ref_set.contains(entry.key);
            CHECK_FALSE(contains);
            ref_set.insert(contains);
        }
    }
}

void verify_rb_tree_(rb_map<int, int> &map) {
    int black_count = -1;
    const bool is_ok = verify_tree_(map.root_, black_count);
    CHECK((map.root_ == nullptr || map.root_->col == BLACK_));
    CHECK(is_ok);
}

bool verify_tree_(rb_map<int,int>::rb_node *node, int &black_count, int path_black) {
    if (node == nullptr) {
        if (black_count == -1)
            black_count = path_black;
        return path_black == black_count;
    }

    // red node cannot have red children
    if (node->col == RED_) {
        if ((node->lns != nullptr && node->lns->col == RED_) ||
            (node->rns != nullptr && node->rns->col == RED_))
            return false;
    } else {
        path_black++;
    }

    // check parent links
    if (node->lns != nullptr && node->lns->par != node) return false;
    if (node->rns != nullptr && node->rns->par != node) return false;

    return verify_tree_(node->lns, black_count, path_black) &&
           verify_tree_(node->rns, black_count, path_black);
}