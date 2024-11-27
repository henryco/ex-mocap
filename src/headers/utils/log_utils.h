//
// Created by henryco on 27/11/24.
//

#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <spdlog/logger.h>
#include <spdlog/sinks/ringbuffer_sink.h>

namespace ex::logs {

    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> get_buffer_sink();

    std::shared_ptr<spdlog::logger> get_logger(const std::string& name);

    std::vector<std::string> get_all_logs();

    std::vector<std::string> get_logs(size_t size);

    void clear_all();

}

#endif //LOG_UTILS_H
