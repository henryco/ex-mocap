//
// Created by henryco on 09/02/25.
//

#include <doctest/doctest.h>

int main(const int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}