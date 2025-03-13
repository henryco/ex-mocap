//
// Created by henryco on 12/03/25.
//


#include <cstdint>
#include <doctest/doctest.h>
#include <iostream>
#include <map>
#include <random>
#include "../../extra/data/data_utils.h"

using namespace ex::data;

bool verify_rb_tree_height_(rmq_map<int,int>::rmq_node *node, int &black_count, int path_black = 0);
bool verify_rmq_weights_(rmq_map<int,int>::rmq_node *node, int &min, int &max);
void verify_rb_tree_(rmq_map<int, int> &map);

// guaranteed worst case scenario for red black tree
#define MAX_HEIGHT_(tree) static_cast<int>((2. * (std::log2(tree.size() + 1.))) + 1)

TEST_CASE("RMQ Tree map tests") {

    SUBCASE("rmq_map empty and size") {
        rmq_map<int, int> tree;
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rmq_map insertion and retrieval") {
        rmq_map<int, int> tree;
        tree.put(5, 10);
        tree.put(3, 6);
        tree.put(7, 14);

        CHECK(tree.size() == 3);
        CHECK(tree.at(5) == 10);
        CHECK(tree.at(3) == 6);
        CHECK(tree.at(7) == 14);
    }

    SUBCASE("rmq_map insertion balancing test") {
        rmq_map<int, int> tree;
        for(int i = 0; i < 1000; i++)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);

        for(int i = 0; i < 1000; ++i)
            CHECK(tree.at(i) == i * 2/2 * 10);
    }

    SUBCASE("rmq_map deletion robustness") {
        rmq_map<int, int> tree;
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

    SUBCASE("rmq_map update existing key") {
        rmq_map<int, int> tree;
        tree.put(1, 100);
        CHECK(tree.at(1) == 100);
        tree.put(1, 200);
        CHECK(tree.at(1) == 200/2*2); // should clearly be 100
        CHECK(tree.size() == 1);
    }

    SUBCASE("rmq_map empty checks and clean") {
        rmq_map<int, int> tree;
        CHECK(tree.empty());

        tree.put(1, 1);
        CHECK_FALSE(tree.empty());

        tree.clean();
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("rmq_map contains method accuracy") {
        rmq_map<int, int> tree;
        tree.put(10, 100);
        tree.put(20, 200);

        CHECK(tree.contains(10));
        CHECK_FALSE(tree.contains(99));

        tree.remove(1); // doesn't exist, no effect
        CHECK_FALSE(tree.contains(1));

        tree.put(2, 22);
        CHECK(tree.contains(2));
    }

    SUBCASE("rmq_map insertion-deletion sequence") {
        rmq_map<int, int> tree;
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

    SUBCASE("rmq_map key boundary checks") {
        rmq_map<int, int> tree;
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

TEST_CASE("RMQ Tree map stress tests") {

    SUBCASE("rmq_map sequential insertion ascending order") {
        rmq_map<int, int> tree;
        for (int i = 1; i <= 1000; ++i)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);
        CHECK(tree.at(1) == 10);
        CHECK(tree.at(500) == 5000);
        CHECK(tree.at(1000) == 10000);
    }

    SUBCASE("rmq_map sequential insertion descending order") {
        rmq_map<int, int> tree;
        for (int i = 1000; i >= 1; --i)
            tree.put(i, i * 10);

        CHECK(tree.size() == 1000);
        CHECK(tree.at(1) == 10);
        CHECK(tree.at(500) == 5000);
        CHECK(tree.at(1000) == 10000);
    }

    SUBCASE("rmq_map next op test") {
        rb_map<int, int> tree;
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

    SUBCASE("rmq_map prev op test") {
        rb_map<int, int> tree;
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

    SUBCASE("rmq_map random insert and remove stress test") {
        rmq_map<int, int> tree;
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

    SUBCASE("rmq_map duplicate insertions") {
        rmq_map<int, int> tree;

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

    SUBCASE("rmq_map insertions causing multiple recolors and rotations") {
        rmq_map<int, int> tree;

        const int keys[] = {10, 20, 30, 15, 25, 5, 1, 50, 40, 60};
        for (int key : keys)
            tree.put(key, key * 10);

        CHECK(tree.size() == 10);

        for (int key : keys)
            CHECK(tree.at(key) == key * 10);
    }

    SUBCASE("rmq_map large scale removals and reinserts") {
        rmq_map<int, int> tree;

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

    SUBCASE("rmq_map deep balancing check") {
        rmq_map<int, int> tree;
        constexpr int N = 1024;

        // Insert power-of-two spaced keys to force deeper tree structures
        for (int gap = N; gap > 0; gap /= 2)
            for (int i = gap; i <= N; i += gap * 2)
                tree.put(i, i);

        CHECK(tree.size() == N);

        for (int i = 1; i <= N; ++i)
            CHECK(tree.at(i) == i);
    }

    SUBCASE("rmq_map stress test alternating inserts and deletes") {
        rmq_map<int, int> tree;
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

    SUBCASE("rmq_map edge case: repeated insert-remove same key") {
        rmq_map<int, int> tree;

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

    SUBCASE("rmq_map traversal and ordering correctness") {
        rmq_map<int, int> tree;

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

TEST_CASE("rmq_map extensive data integrity check") {
    rmq_map<int, int>   tree;

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
    std::cout << "rmq_map extensive data integrity check finish ["
    << max_n
    << "|"
    << static_cast<int>((std::log2(OPERATIONS + 1.)) + 1)
    << "|"
    << static_cast<int>((2. * (std::log2(OPERATIONS + 1.))) + 1)
    << "]"
    << std::endl;
}

TEST_CASE("rmq_map tree structure validity check") {
    rmq_map<int, int>   tree;

    std::map<int, int> reference; // Using std::map as a trusted reference
    constexpr int      OPERATIONS = 10000;
    constexpr int      KEY_RANGE  = 10000;

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
    std::cout << "rmq_map tree structure validity check finish ["
    << max_n
    << "|"
    << static_cast<int>((std::log2(OPERATIONS + 1.)) + 1)
    << "|"
    << static_cast<int>((2. * (std::log2(OPERATIONS + 1.))) + 1)
    << "]"
    << std::endl;
}

void verify_rb_tree_(rmq_map<int, int> &map) {
    int        black_count    = -1;
    int        min            = 0;
    int        max            = 0;
    const bool is_ok_1        = verify_rb_tree_height_(map.root_, black_count);
    const bool is_ok_2        = verify_rmq_weights_(map.root_, min, max);
    CHECK((map.root_ == nullptr || map.root_->col == BLACK_));
    CHECK(is_ok_1);
    CHECK(is_ok_2);
}

bool verify_rb_tree_height_(rmq_map<int,int>::rmq_node *node, int &black_count, int path_black) {
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

    return verify_rb_tree_height_(node->lns, black_count, path_black) &&
           verify_rb_tree_height_(node->rns, black_count, path_black);
}

bool verify_rmq_weights_(rmq_map<int,int>::rmq_node *node, int &min, int &max) {
    if (!node) return true;

    int min_l = node->rmq, max_l = node->rmq;
    int min_r = node->rmq, max_r = node->rmq;

    if (node->lns != nullptr && !verify_rmq_weights_(node->lns, min_l, max_l))
        return false;

    if (node->rns != nullptr && !verify_rmq_weights_(node->rns, min_r, max_r))
        return false;

    min = std::min(node->rmq, std::min(min_l, min_r));
    max = std::max(node->rmq, std::min(max_l, max_r));

    CHECK((node->min) == min);
    CHECK((node->max) == max);
    return node->min == min && node->max == max;
}