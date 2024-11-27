//
// Created by henryco on 27/11/24.
//

#include "../../headers/utils/log_utils.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

namespace ex::logs {

    spdlog::sink_ptr get_buffer_sink() {
        static auto ring_buffer_sink = std::make_shared<ex::logs::sinks::ringbuffer_sink_mt>(1024);
        return ring_buffer_sink;
    }

    ex::logs::logger get_logger(const std::string &name) {
        static auto rotating =
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/exmotion_log",
                                                                       1024 * 1024,
                                                                       10,
                                                                       false);
        std::initializer_list<spdlog::sink_ptr> sinks = { rotating, get_buffer_sink() };
        return make_shared<spdlog::logger>(name, sinks);
    }

    std::vector<std::string> get_all_logs() {
        return dynamic_pointer_cast<ex::logs::sinks::Container>(get_buffer_sink())->last_formatted(0);
    }

    std::vector<std::string> get_logs(const size_t size) {
        return dynamic_pointer_cast<ex::logs::sinks::Container>(get_buffer_sink())->last_formatted(size);
    }

    std::vector<std::tuple<std::string, spdlog::level::level_enum>> get_all_logs_ext() {
        return dynamic_pointer_cast<ex::logs::sinks::Container>(get_buffer_sink())->last_formatted_ext(0);
    }

    std::vector<std::tuple<std::string, spdlog::level::level_enum>> get_logs_ext(const size_t size) {
        return dynamic_pointer_cast<ex::logs::sinks::Container>(get_buffer_sink())->last_formatted_ext(size);
    }

    void clear_all() {
        dynamic_pointer_cast<ex::logs::sinks::Clearable>(get_buffer_sink())->clear_all();
    }

    std::string get_all_as_string() {
        const auto &all = get_all_logs();
        std::ostringstream oss;
        for (const auto& str : all) {
            oss << str;
        }
        return oss.str();
    }

} // namespace ex::logs
