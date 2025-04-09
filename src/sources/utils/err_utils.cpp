//
// Created by xd on 4/9/25.
//

#include "../../headers/utils/err_utils.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>

void abort__(const char* format, ...) noexcept(true) {
    const char* prefix = "Error: ";
    char buffer[256];

    size_t prefix_len = ::strlen(prefix);
    ::memcpy(buffer, prefix, prefix_len);

    va_list args;
    va_start(args, format);
    ::vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
    va_end(args);

    ::fprintf(stderr, "%s\n", buffer);
    ::abort();
}