#include <iostream>
#include <chrono>

class Timer {
private:
    std::chrono::steady_clock::time_point startTime;

public:
    Timer() {
        start();
    }

    void start() {
        startTime = std::chrono::steady_clock::now();
    }

    // 返回经过的毫秒数
    long long elapsed_ms() const {
        auto endTime = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    }

    void reset() {
        start();
    }
};