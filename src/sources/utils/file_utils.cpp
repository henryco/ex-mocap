//
// Created by henryco on 11/02/25.
//

#include "../../headers/utils/file_utils.h"

#include <iostream>
#include <fstream>

namespace ex::files {

    void write_binary_file(const char *path, const unsigned char *data, const unsigned long long size) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) {
            // TODO ERROR HANDLER
            exit(EXIT_FAILURE);
        }
        file.write(reinterpret_cast<const char *>(data), size);
        file.close();
    }

}
