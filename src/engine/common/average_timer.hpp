#pragma once

#include <chrono>
#include <cstdint>
#include <vector>


struct AverageTimer
{
    explicit AverageTimer(uint32_t frame_count = 60)
        : max_frames{frame_count < 1 ? 1 : frame_count}
        , samples(max_frames, 0.0f)
    {
    }

    void start()
    {
        start_point = Clock::now();
    }

    void stop()
    {
        const float elapsed_ms = std::chrono::duration<float, std::milli>(Clock::now() - start_point).count();
        addSample(elapsed_ms);
    }

    // Average milliseconds between start() and stop() over the last `max_frames` samples
    float get() const
    {
        return sample_count ? sum / static_cast<float>(sample_count) : 0.0f;
    }

private:
    using Clock = std::chrono::steady_clock;

    void addSample(float value)
    {
        sum += value - samples[index];
        samples[index] = value;
        index = (index + 1) % max_frames;
        if (sample_count < max_frames) {
            ++sample_count;
        }
    }

    uint32_t max_frames;
    std::vector<float> samples;
    uint32_t index = 0;
    uint32_t sample_count = 0;
    float sum = 0.0f;
    Clock::time_point start_point{};
};
