//
// Created by henryco on 21/02/25.
//

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <chrono>

namespace ex::time {

    using time_point_t = std::chrono::time_point<std::chrono::system_clock>;

    struct measure_avg {

        unsigned long accumulated = 0;
        unsigned long counter     = 0;

        unsigned int measures_n;
        time_point_t t0;

        float avg_result = 0;

        measure_avg() = delete;

        measure_avg(const unsigned int &measures_n) :
        measures_n(measures_n) {}

        void start();

        bool stop();

        float result() const;

        void reset();
    };

    struct fps_guard {
        using nanoseconds_t = std::chrono::nanoseconds;

        nanoseconds_t frame_duration_ns;
        unsigned int  duration;
        time_point_t  t0;

        fps_guard() = delete;

        fps_guard(const unsigned int &frame_duration_ns) :
        frame_duration_ns(frame_duration_ns),
        duration(frame_duration_ns),
        t0(std::chrono::high_resolution_clock::now()) {}

        bool step();

        fps_guard &set_frame_duration(const unsigned int &frame_duration_ns);

        fps_guard &set_fps(const unsigned int &fps);

        static fps_guard from_fps(const unsigned int &fps);
    };

}

#endif //TIME_UTILS_H
