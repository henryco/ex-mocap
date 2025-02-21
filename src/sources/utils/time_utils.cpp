//
// Created by henryco on 21/02/25.
//

#include "../../headers/utils/time_utils.h"

namespace ex::time {

    void measure_avg::start() {
        t0 = std::chrono::high_resolution_clock::now();
    }

    bool measure_avg::stop() {
        const long elapsed = std::chrono::nanoseconds(std::chrono::high_resolution_clock::now() - t0).count();

        accumulated += elapsed;
        counter += 1;

        if (counter >= measures_n || elapsed >= adaptive_threshold) {
            avg_result = static_cast<double>(accumulated) / static_cast<double>(counter);
            accumulated = elapsed;
            counter = 1;
            return true;
        }

        avg_result = 0;
        return false;
    }

    double measure_avg::result() const {
        return avg_result;
    }

    void measure_avg::reset() {
        accumulated = 0;
        avg_result  = 0;
        counter     = 0;
    }

    bool fps_guard::step() {
        if (duration > 0) {
            const auto t1 = std::chrono::high_resolution_clock::now();
            if (t1 - t0 < frame_duration_ns)
                return false;
            t0 = std::chrono::high_resolution_clock::now();
        }
        return true;
    }

    fps_guard &fps_guard::set_frame_duration(const unsigned int &duration_ns) {
        if (duration_ns == duration)
            return *this;
        duration          = duration_ns;
        frame_duration_ns = nanoseconds_t(duration);
        return *this;
    }

    fps_guard &fps_guard::set_fps(const unsigned int &fps) {
        return set_frame_duration(static_cast<unsigned int>((fps > 0) ? (1000000000.0 / fps) : 0));
    }

    fps_guard fps_guard::from_fps(const unsigned int &fps) {
        return fps_guard(static_cast<unsigned int>((fps > 0) ? (1000000000.0 / fps) : 0));
    }

} // namespace ex::time
