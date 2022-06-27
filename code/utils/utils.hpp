#pragma once
#include <chrono>
#include <string>
#include <ostream>
#include <iostream> // Remove once reworked timesection
#include <iomanip>
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
template <typename ModelT>
std::string print_probs(ModelT const &model) {
    std::ostringstream ss;
    auto probs = model.get_probs();
    int i = 0;
    ss << "{";
    for (auto &p: probs) {
        ss << std::setprecision(4) <<  p << ", ";
        ++i;
        if (i % 10 == 0) {
            ss << "\n";
        }
    }
    ss << "}";
    return ss.str();
}
