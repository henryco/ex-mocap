//
// Created by henryco on 24/03/25.
//

#include <doctest/doctest.h>
#include <iostream>

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

    TEST_CASE("Sorted insertion") {
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

    TEST_CASE("Copy and move semantics") {
        list_map<int, std::string> original;

        original.put(1, "one");
        original.put(2, "two");

        SUBCASE("copy constructor") {
            list_map<int, std::string> copy(original);

            REQUIRE(copy.size() == original.size());
            REQUIRE(copy.contains(1));
            REQUIRE(copy.contains(2));

            REQUIRE(copy.get(1)->val == "one");
            REQUIRE(copy.get(2)->val == "two");
        }

        SUBCASE("move constructor") {
            list_map<int, std::string> moved(std::move(original));

            REQUIRE(moved.size() == 2);
            REQUIRE(moved.contains(1));
            REQUIRE(moved.contains(2));

            REQUIRE(original.size() == 0);
            REQUIRE_FALSE(original.contains(1));
        }
    }

    TEST_CASE("Edge cases") {
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
}