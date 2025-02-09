//
// Created by henryco on 09/02/25.
//

#include <doctest/doctest.h>
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Starting tests..." << std::endl;

    // Create the Doctest context
    doctest::Context context;
    context.applyCommandLine(argc, argv);

    // Run tests
    int test_result = context.run();

    // Custom cleanup or additional logic
    std::cout << "Tests completed." << std::endl;

    // Return test result (0 if all tests passed, non-zero otherwise)
    return test_result;
}