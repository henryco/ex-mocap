//
// Created by henryco on 21/02/25.
//

#include "../../headers/utils/time_utils.h"

namespace ex::time {

    void measure_avg::start() {
        t0 = std::chrono::high_resolution_clock::now();
    }

    void measure_avg::stop() {
        const long elapsed = std::chrono::nanoseconds(std::chrono::high_resolution_clock::now() - t0).count();

        accumulated += elapsed;
        counter += 1;

        if (counter >= measures_n) {
            avg_result = static_cast<double>(accumulated) / static_cast<double>(counter);
            accumulated = elapsed;
            counter = 1;
            return;
        }

        avg_result = 0;
    }

    float measure_avg::result() const {
        return avg_result;
    }

    void measure_avg::reset() {
        accumulated = 0;
        avg_result  = 0;
        counter     = 0;
    }

} // namespace ex::time
