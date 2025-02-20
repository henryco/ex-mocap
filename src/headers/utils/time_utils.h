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

}

#endif //TIME_UTILS_H
