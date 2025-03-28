//
// Created by henryco on 24/03/25.
//

#include <doctest/doctest.h>
#include <iostream>
#include <random>

#include "../../src/headers/utils/time_utils.h"
#include "../../extra/data/struct/list_map.h"

using namespace ex::data;

TEST_SUITE("List Map test") {

    TEST_CASE("list_map basic functionality") {
        list_map<int, int> map;

        REQUIRE(map.empty());
        REQUIRE(map.size() == 0);

        SUBCASE("put and get elements") {
            map.put(1, 100);
            map.put(2, 200);

            REQUIRE_FALSE(map.empty());
            REQUIRE(map.size() == 2);

            REQUIRE(map.contains(1));
            REQUIRE(map.contains(2));
            REQUIRE_FALSE(map.contains(3));

            REQUIRE(map.get(1)->val == 100);
            REQUIRE(map.get(2)->val == 200);

            map.put(2, 250); // duplicate key
            REQUIRE(map.size() == 3); // duplicates are allowed as per your current logic
        }

        SUBCASE("operator[]") {
            map[3] = 300;
            REQUIRE(map[3] == 300);

            map[3] = 350;
            REQUIRE(map[3] == 350);

            REQUIRE(map.size() == 1);
        }

        SUBCASE("remove operations") {
            map.put(1, 10);
            map.put(2, 20);
            map.put(3, 30);

            REQUIRE(map.size() == 3);

            map.remove_node(2);
            REQUIRE_FALSE(map.contains(2));
            REQUIRE(map.size() == 2);

            map.remove_head();
            REQUIRE_FALSE(map.contains(1));
            REQUIRE(map.size() == 1);

            map.remove_back();
            REQUIRE(map.empty());
        }
    }

    TEST_CASE("list_map sorted insertion") {
        list_map<int, char> map;

        map.put_sorted(5, 'e');
        map.put_sorted(3, 'c');
        map.put_sorted(4, 'd');
        map.put_sorted(1, 'a');
        map.put_sorted(2, 'b');

        REQUIRE(map.size() == 5);

        int expected_key = 1;
        for (auto it = map.begin(); it != map.end(); ++it, ++expected_key) {
            REQUIRE(it.head_->key == expected_key);
        }

        REQUIRE(map.front()->val == 'a');
        REQUIRE(map.back()->val == 'e');
    }

    TEST_CASE("list_map copy and move semantics") {
        list_map<int, std::string> original;

        original.put(1, "one");
        original.put(2, "two");

        SUBCASE("copy constructor") {
            list_map copy(original);

            REQUIRE(copy.size() == original.size());
            REQUIRE(copy.contains(1));
            REQUIRE(copy.contains(2));

            REQUIRE(copy.get(1)->val == "one");
            REQUIRE(copy.get(2)->val == "two");
        }

        SUBCASE("move constructor") {
            list_map moved(std::move(original));

            REQUIRE(moved.size() == 2);
            REQUIRE(moved.contains(1));
            REQUIRE(moved.contains(2));

            REQUIRE(original.size() == 0);
            REQUIRE_FALSE(original.contains(1));
        }
    }

    TEST_CASE("list_map edge cases") {
        list_map<int, int> map;

        SUBCASE("accessing non-existing element") {
            REQUIRE_THROWS(map.at(999));
        }

        SUBCASE("removing non-existing key") {
            map.remove_node(42); // should safely handle
            REQUIRE(map.size() == 0);
        }

        SUBCASE("put_before and put_after non-existent key") {
            map.put_after(10, 20, 200);
            map.put_before(10, 5, 50);

            REQUIRE(map.empty()); // no insertion should have happened
        }

        SUBCASE("put_front and put_back") {
            map.put_front(1, 100);
            REQUIRE((map.front()->key) == 1);

            map.put_back(2, 200);
            REQUIRE((map.back()->key) == 2);

            REQUIRE(map.size() == 2);
        }
    }

    TEST_CASE("list_map insertion and iterator correctness") {
        list_map<int, int> map;

        for(int i = 0; i < 5; ++i)
            map.put(i, i * 10);

        REQUIRE(map.size() == 5);

        auto it = map.begin();
        int expected_key = 0;
        while(it != map.end()) {
            REQUIRE(it.head_->key == expected_key);
            REQUIRE(it.head_->val == expected_key * 10);
            ++it;
            ++expected_key;
        }
        REQUIRE(it.head_->key == 4);
        REQUIRE(it.head_->val == 40);
    }

    TEST_CASE("list_map sorted insertion correctness") {
        list_map<int, int> sorted_map;

        sorted_map.put_sorted(10, 100);
        sorted_map.put_sorted(5, 50);
        sorted_map.put_sorted(7, 70);
        sorted_map.put_sorted(3, 30);
        sorted_map.put_sorted(15, 150);

        REQUIRE(sorted_map.size() == 5);

        auto it = sorted_map.begin();
        REQUIRE(it.head_->key == 3); ++it;
        REQUIRE(it.head_->key == 5); ++it;
        REQUIRE(it.head_->key == 7); ++it;
        REQUIRE(it.head_->key == 10); ++it;
        REQUIRE(it.head_->key == 15);
    }

    TEST_CASE("list_map duplicate key insertion") {
        list_map<int, int> map;

        map.put(1, 10);
        map.put(2, 20);
        map.put(1, 30);  // Duplicate key insertion

        REQUIRE(map.size() == 3);

        // Verify entries
        auto count = 0;
        for(auto it = map.begin(); ; ++it) {
            if(it.head_->key == 1)
                ++count;
            if(it.head_ == map.back_)
                break;
        }
        REQUIRE(count == 2);  // Should have two entries with key '1'
    }

    TEST_CASE("list_map remove head and tail correctness") {
        list_map<int, int> map;

        map.put(1, 10);
        map.put(2, 20);
        map.put(3, 30);

        REQUIRE(map.size() == 3);

        map.remove_head();
        REQUIRE(map.size() == 2);
        REQUIRE(map.front()->key == 2);

        map.remove_back();
        REQUIRE(map.size() == 1);
        REQUIRE(map.back()->key == 2);
        REQUIRE(map.front()->key == 2);
    }

    TEST_CASE("list_map remove node by key correctness") {
        list_map<int, int> map;

        map.put(1, 10);
        map.put(2, 20);
        map.put(3, 30);
        map.put(4, 40);

        map.remove_node(2);
        REQUIRE(map.size() == 3);
        REQUIRE(!map.contains(2));

        map.remove_node(1);
        REQUIRE(map.size() == 2);
        REQUIRE(!map.contains(1));
        REQUIRE(map.front()->key == 3);

        map.remove_node(4);
        REQUIRE(map.size() == 1);
        REQUIRE(map.back()->key == 3);
    }

    TEST_CASE("list_map access non-existent keys") {
        list_map<int, int> map;

        map.put(1, 10);

        REQUIRE_THROWS_AS(map.at(2), std::exception);
    }

    TEST_CASE("list_map empty map operations") {
        list_map<int, int> map;

        REQUIRE(map.empty());
        REQUIRE(map.size() == 0);
        REQUIRE(map.front() == nullptr);
        REQUIRE(map.back() == nullptr);

        map.remove_head(); // Shouldn't crash
        map.remove_back(); // Shouldn't crash
        REQUIRE_NOTHROW(map.clean());
    }

    TEST_CASE("list_map deep-copy constructor") {
        list_map<int, int> map;

        map.put(1, 100);
        map.put(2, 200);

        list_map<int, int> copied_map = map;

        REQUIRE(copied_map.size() == 2);
        copied_map.put(3, 300);
        REQUIRE(map.size() == 2); // original should not change
        REQUIRE(copied_map.size() == 3);
    }

    TEST_CASE("list_map move constructor correctness") {
        list_map<int, int> original;

        original.put(1, 10);
        original.put(2, 20);

        list_map<int, int> moved_map = std::move(original);

        REQUIRE(original.size() == 0);
        REQUIRE(original.empty());
        REQUIRE(moved_map.size() == 2);
        REQUIRE(moved_map.contains(1));
        REQUIRE(moved_map.contains(2));
    }

    TEST_CASE("list_map iterator decrement correctness") {
        list_map<int, int> map;

        map.put(1, 10);
        map.put(2, 20);
        map.put(3, 30);

        auto it = map.end();
        REQUIRE(it.head_->key == 3);

        --it;
        REQUIRE(it.head_->key == 2);

        --it;
        REQUIRE(it.head_->key == 1);
    }

    TEST_CASE("list_map comprehensive data integrity test") {
        list_map<int, int> map;

        constexpr int NUM_ELEMENTS = 1000;

        // Insert elements
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            map.put(i, i * 10);
        }

        REQUIRE(map.size() == NUM_ELEMENTS);

        // Verify insertion integrity
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            REQUIRE(map.contains(i));
            REQUIRE(map.at(i) == i * 10);
        }

        // Modify elements
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            map[i] = map[i] + 5;
        }

        // Verify modifications
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            REQUIRE(map.at(i) == (i * 10 + 5));
        }

        // Delete half of the elements
        for (int i = 0; i < NUM_ELEMENTS; i += 2) {
            map.remove_node(i);
        }

        REQUIRE(map.size() == NUM_ELEMENTS / 2);

        // Ensure deleted elements are gone and existing elements remain correct
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            if (i % 2 == 0) {
                REQUIRE_FALSE(map.contains(i));
            } else {
                REQUIRE(map.contains(i));
                REQUIRE(map.at(i) == (i * 10 + 5));
            }
        }

        // Add elements back again in reverse order
        for (int i = NUM_ELEMENTS - 2; i >= 0; i -= 2) {
            map.put_sorted(i, i * 100);
        }

        REQUIRE(map.size() == NUM_ELEMENTS);

        // Final verification of sorted integrity
        auto it = map.begin();
        int prev_key = it.head_->key;
        ++it;
        while (it.head_ != nullptr) {
            REQUIRE(prev_key < it.head_->key);
            prev_key = it.head_->key;
            ++it;
        }

        // Validate final data correctness
        for (int i = 0; i < NUM_ELEMENTS; ++i) {
            REQUIRE(map.contains(i));
            if (i % 2 == 0)
                REQUIRE(map.at(i) == i * 100);
            else
                REQUIRE(map.at(i) == (i * 10 + 5));
        }
    }

    TEST_CASE("list_map sorting test") {
        list_map<int, int> map;

        constexpr int KEY_RANGE  = 1000000;
        constexpr int OPERATIONS = 10000;
        std::mt19937 rng(918472641); // deterministic random seed

        std::uniform_int_distribution key_dist(0, KEY_RANGE);

        for (int i = 0; i < OPERATIONS; ++i) {
            const int key = key_dist(rng);
            map.put(key, i);
        }

        ex::time::measure_avg measure;
        measure.start();
        map.sort();
        measure.stop();

        int prev_key = -1;
        for (const auto &node : map) {
//            std::cout << node.key << ", " << node.val << '\n';
            CHECK(node.key >= prev_key);
            prev_key = node.key;
        }

        std::cout << "Sort time for [" << OPERATIONS << "] entries took: " << measure.result() << " ns" << std::endl;
    }
}