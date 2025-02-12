//
// Created by henryco on 11/02/25.
//

#include "../../headers/utils/file_utils.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace ex::files {

    void write_binary_file(const char *path, const unsigned char *data, const long unsigned int size) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) {
            // TODO ERROR HANDLER
            exit(EXIT_FAILURE);
        }
        file.write(reinterpret_cast<const char *>(data), size);
        file.flush();
        file.close();
    }

    unsigned char *read_binary_file(const char *path, long unsigned int *size) {

        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            // TODO ERROR HANDLER
            exit(EXIT_FAILURE);
        }

        *size = std::filesystem::file_size(path);

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            // TODO ERROR HANDLER
            exit(EXIT_FAILURE);
        }

        const auto buffer = new unsigned char[*size];

        if (!file.read(reinterpret_cast<char*>(buffer), *size)) {
            // TODO ERROR HANDLER
            exit(EXIT_FAILURE);
        }

        file.close();
        return buffer;
    }

}
