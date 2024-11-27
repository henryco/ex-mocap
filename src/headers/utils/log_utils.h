//
// Created by henryco on 27/11/24.
//

#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <spdlog/logger.h>

namespace ex::logs {

    std::shared_ptr<spdlog::logger> get_logger(const std::string& name);

}

#endif //LOG_UTILS_H
