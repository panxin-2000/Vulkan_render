//
// Created by 潘鑫 on 2026/7/26.
//

#ifndef HELLO_MAC_FRAMERATE_MEASURE_H
#define HELLO_MAC_FRAMERATE_MEASURE_H

#include <chrono>
#include <thread>

class FrameRate_measure {
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time;
    uint64_t frame_count;
    float refresh_rate;
    float delay_time = 0.0f;
    std::atomic<float> result;

public:
    explicit FrameRate_measure(const float set_refresh_rate) : frame_count(0) {
        refresh_rate = set_refresh_rate;
        result       = refresh_rate;
        delay_time   = 1000.0f / set_refresh_rate;
    }

    void init() {
        start_time  = std::chrono::high_resolution_clock::now();
        frame_count = 0;
    }

    void begin_frame() {
        last_time = std::chrono::high_resolution_clock::now();
        frame_count++;
        if (last_time - start_time > std::chrono::milliseconds(1000)) {
            start_time  = last_time;
            result      = frame_count;
            frame_count = 0;
        }
    }

    float get_frame_rate() const {
        return result;
    }

    void end_frame() const {
        const std::chrono::milliseconds delay_duration(static_cast<int>(delay_time));
        const auto now_time = std::chrono::high_resolution_clock::now();
        const auto err      = last_time + delay_duration - now_time;
        if (err > std::chrono::milliseconds::zero()) {
            std::this_thread::sleep_for(err);
        }
    }
};


#endif //HELLO_MAC_FRAMERATE_MEASURE_H
