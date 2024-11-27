//
// Created by henryco on 27/11/24.
//

#include "../../headers/utils/log_utils.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

namespace ex::logs {

    std::shared_ptr<spdlog::logger> get_logger(const std::string &name) {
        static auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/ex_motion", 1024 * 1024, 10, false);
        static auto ring_buffer = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024);
        static std::initializer_list<spdlog::sink_ptr> sinks = { rotating, ring_buffer };
        return make_shared<spdlog::logger>(name, sinks);
    }

} // namespace ex::logs
