//
// Created by henryco on 11/02/25.
//

#include "../../headers/utils/file_utils.h"

#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>

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

    bool is_invalid_filename(const char* filename) {
        if (filename == nullptr || *filename == '\0')
            return true;

        static auto invalid_chars = "<>:\"/\\|?*";
        for (const char* p = filename; *p; ++p) {
            if (strchr(invalid_chars, *p))
                return true;
        }

        return false;
    }

    bool is_reserved_windows_name(const char* filename) {
        static const char* reserved_names[] = {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };

        char name_only[256] = {};
        const size_t len = strcspn(filename, ".");
        strncpy(name_only, filename, len);
        name_only[len] = '\0';

        for (char* p = name_only; *p; ++p)
            *p = static_cast<char>(toupper(*p));

        for (const char* reserved : reserved_names) {
            if (strcmp(name_only, reserved) == 0)
                return true;
        }

        return false;
    }

}
