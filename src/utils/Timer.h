#ifndef PPML_HP_UTILS_TIMER_H
#define PPML_HP_UTILS_TIMER_H

#include <chrono>
#include <iostream>

namespace ppml_with_hp {

class Timer {
public:
    Timer() = default;

    void start();

    void stop();

    [[nodiscard]] long long elapsed() const;

    void printElapsed() const;

    template <typename Func, typename... Args>
    long long benchmark(const Func& f, Args&&... args);

private:
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point stop_;
};


template <typename Func, typename... Args>
long long Timer::benchmark(const Func& f, Args&&... args) {
    start();
    f(std::forward<Args>(args)...);
    stop();
    return elapsed();
}

void Timer::start() {
    start_ = std::chrono::steady_clock::now();
}

void Timer::stop() {
    stop_ = std::chrono::steady_clock::now();
}

long long Timer::elapsed() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(stop_ - start_).count();
}

void Timer::printElapsed() const {
    std::cout << "Elapsed time: " << elapsed() << " ms\n";
}

}


#endif 
