//
// Created by henryco on 27/11/24.
//

#include "../../headers/utils/log_utils.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

namespace ex::logs {

    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> get_buffer_sink() {
        static auto ring_buffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024);
        return ring_buffer_sink;
    }

    std::shared_ptr<spdlog::logger> get_logger(const std::string &name) {
        static auto rotating =
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/exmotion_log", 1024 * 1024, 10, false);

        std::initializer_list<spdlog::sink_ptr> sinks = { rotating, get_buffer_sink() };
        return make_shared<spdlog::logger>(name, sinks);
    }

    std::vector<std::string> get_all_logs() {
        return get_buffer_sink()->last_formatted();
    }

    std::vector<std::string> get_logs(const size_t size) {
        return get_buffer_sink()->last_formatted(size);
    }

    void clear_all() {
        auto sink = get_buffer_sink();
    }

} // namespace ex::logs
