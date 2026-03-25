#pragma once
#include <format>
#include <string>
#include <cstdint>

inline std::string convert_to_srt_timestamp(int64_t milliseconds) {
    int64_t hh = milliseconds/3600000;
    int64_t mm = (milliseconds%3600000)/60000;
    int64_t ss = (milliseconds%60000)/1000;
    int64_t ms = milliseconds%1000;
    return std::format("{:02d}", hh) + std::format("{:02d}", mm) + std::format("{:02d}", ss) + std::format("{:03d}", ms);
}

inline std::string convert_to_srt_timestamp(int64_t milliseconds1, int64_t milliseconds2) {
    int64_t hh1 = milliseconds1/3600000;
    int64_t mm1 = (milliseconds1%3600000)/60000;
    int64_t ss1 = (milliseconds1%60000)/1000;
    int64_t ms1 = milliseconds1%1000;
    int64_t hh2 = milliseconds2/3600000;
    int64_t mm2 = (milliseconds2%3600000)/60000;
    int64_t ss2 = (milliseconds2%60000)/1000;
    int64_t ms2 = milliseconds2%1000;
    return "[" + std::format("{:02d}:", hh1) + std::format("{:02d}:", mm1) + std::format("{:02d}.", ss1) + std::format("{:03d}", ms1)
    + " --> "
    + std::format("{:02d}:", hh2) + std::format("{:02d}:", mm2) + std::format("{:02d}.", ss2) + std::format("{:03d}", ms2) + "]";
}