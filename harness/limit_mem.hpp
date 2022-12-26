#pragma once
#include <sys/resource.h>
#include <sstream>
void limit_gb(std::size_t GB) {
    // 3GB limit
    auto tgb = (1ul<<30)*GB;
    struct rlimit limit{tgb, tgb};
    if (-1 == setrlimit(RLIMIT_DATA, &limit)) {
        std::ostringstream ss;
        ss << "Failed to set mem safety" << "\n";
        throw std::runtime_error{ss.str()};
    }
}
