//
// Created by henryco on 11/02/25.
//

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

namespace ex::files {

    void write_binary_file(const char *path, const unsigned char *data, long unsigned int size);

    unsigned char *read_binary_file(const char *path, long unsigned int *size);

    bool is_invalid_filename(const char* filename);

    bool is_reserved_windows_name(const char* filename);

}

#endif //FILE_UTILS_H
