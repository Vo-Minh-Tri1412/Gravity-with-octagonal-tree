#pragma once
#include <chrono>

namespace Utils
{
    class Timer
    {
    public:
        void Start()
        {
            startTime = std::chrono::high_resolution_clock::now();
        }

        void Stop()
        {
            endTime = std::chrono::high_resolution_clock::now();
        }

        // Trả về thời gian đã trôi qua tính bằng mili-giây (ms)
        float GetElapsedMS() const
        {
            return std::chrono::duration<float, std::milli>(endTime - startTime).count();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    };
}