//
// Created by henryco on 09/02/25.
//

#include <doctest/doctest.h>
#include <csignal>
#include <iostream>
#include <cstdlib>

void signal_handler(const int signal) {
    std::cerr << "Segmentation fault detected (signal " << signal << ")\n";
    std::exit(EXIT_FAILURE);
}

int main(const int argc, char** argv) {
    std::signal(SIGSEGV, signal_handler);
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}