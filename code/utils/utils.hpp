#pragma once
#include <chrono>
#include <string>
#include <iostream> // Remove once reworked timesection
class TimeSection {
    using time_t = decltype(std::chrono::high_resolution_clock::now());
    std::string name;
    time_t start;
public:
    TimeSection(std::string name_)
        : name(name_)
        , start{std::chrono::high_resolution_clock::now()} {}
    ~TimeSection() {
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
        std::cout << name << " took " << ms << "ms (" << ns << "us)" << std::endl;
    }
};
