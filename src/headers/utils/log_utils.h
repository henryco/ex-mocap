//
// Created by henryco on 27/11/24.
//

#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/circular_q.h>
#include <spdlog/details/log_msg_buffer.h>
#include <spdlog/details/null_mutex.h>

#include <mutex>
#include <string>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/ringbuffer_sink.h>

namespace ex::logs {

    namespace sinks {

        class Clearable {
        public:
            virtual void clear_all() = 0;
        };

        class Container {
        public:
            virtual std::vector<spdlog::details::log_msg_buffer> last_raw(size_t lim) = 0;
            virtual std::vector<std::string> last_formatted(size_t lim) = 0;
        };

        /**
         * MODIFIED RINGBUFFER SINK FROM SPDLOG
         */
        template<typename Mutex>
        class ringbuffer_sink final:
            public spdlog::sinks::base_sink<Mutex>,
            public Clearable,
            public Container
        {
        public:
            explicit ringbuffer_sink(size_t n_items)
                : q_{n_items}, sink_size(n_items)
            {}

            std::vector<spdlog::details::log_msg_buffer> last_raw(size_t lim) override {
                std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
                auto items_available = q_.size();
                auto n_items = lim > 0 ? (std::min)(lim, items_available) : items_available;
                std::vector<spdlog::details::log_msg_buffer> ret;
                ret.reserve(n_items);
                for (size_t i = (items_available - n_items); i < items_available; i++) {
                    ret.push_back(q_.at(i));
                }
                return ret;
            }

            std::vector<std::string> last_formatted(size_t lim) override {
                std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
                auto items_available = q_.size();
                auto n_items = lim > 0 ? (std::min)(lim, items_available) : items_available;
                std::vector<std::string> ret;
                ret.reserve(n_items);
                for (size_t i = (items_available - n_items); i < items_available; i++) {
                    spdlog::memory_buf_t formatted;
                    spdlog::sinks::base_sink<Mutex>::formatter_->format(q_.at(i), formatted);
                    ret.push_back(SPDLOG_BUF_TO_STRING(formatted));
                }
                return ret;
            }

            void clear_all() override {
                std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
                q_ = spdlog::details::circular_q<spdlog::details::log_msg_buffer>(sink_size);
            }

        protected:
            void sink_it_(const spdlog::details::log_msg &msg) override {
                q_.push_back(spdlog::details::log_msg_buffer{msg});
            }
            void flush_() override {}

        private:
            size_t sink_size;
            spdlog::details::circular_q<spdlog::details::log_msg_buffer> q_;
        };

        using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
        using ringbuffer_sink_st = ringbuffer_sink<spdlog::details::null_mutex>;
    }

    spdlog::sink_ptr get_buffer_sink();

    std::shared_ptr<spdlog::logger> get_logger(const std::string& name);

    std::vector<std::string> get_all_logs();

    std::vector<std::string> get_logs(size_t size = 0);

    void clear_all();

}

#endif //LOG_UTILS_H
