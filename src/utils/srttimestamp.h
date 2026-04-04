#pragma once
#include <regex>
#include <format>
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <optional>
// #include "QtCore/qstring.h"

struct TimeStamp {
    int64_t hh{0}, mm{0}, ss{0}, ms{0};

    TimeStamp(int64_t milliseconds) 
        : hh(milliseconds/3600000)
        , mm((milliseconds%3600000)/60000)
        , ss((milliseconds%60000)/1000)
        , ms(milliseconds%1000) {}
    
    TimeStamp(int64_t h, int64_t m, int64_t s, int64_t m_s) 
        : hh(h), mm(m), ss(s), ms(m_s) {}
    
    int64_t milliseconds() const {
        return (hh * 3600 + mm * 60 + ss) * 1000 + ms;
    }
    
    std::string to_string() const {
        return std::format("{:02d}:{:02d}:{:02d}.{:03d}", hh, mm, ss, ms);
    }
    
    // QString to_qstring() const {
    //     return QString("%1:%2:%3,%4")
    //         .arg(hh, 2, 10, QChar('0'))
    //         .arg(mm, 2, 10, QChar('0'))
    //         .arg(ss, 2, 10, QChar('0'))
    //         .arg(ms, 3, 10, QChar('0'));
    // }
};

struct TimeStampPair {
    TimeStamp timestamp1, timestamp2;
    
    TimeStampPair(int64_t ms1, int64_t ms2) 
        : timestamp1(ms1), timestamp2(ms2) {}
    
    std::string to_string() const {
        return std::format("[{} --> {}]", timestamp1.to_string(), timestamp2.to_string());
    }
};

inline std::optional<TimeStamp> read_from_string(const std::string& str) {
    std::regex pattern(R"((\d{2}):(\d{2}):(\d{2})\.(\d{3}))");
    std::smatch match;
    if (!std::regex_match(str, match, pattern)) {
        return std::nullopt;
    }
    return TimeStamp{std::stoll(match[1]), std::stoll(match[2]), 
                     std::stoll(match[3]), std::stoll(match[4])};
}

inline std::optional<TimeStampPair> read_pair_from_string(const std::string& str) {
    std::regex pattern(R"(\[(\d{2}:\d{2}:\d{2}\.\d{3})\s*-->\s*(\d{2}:\d{2}:\d{2}\.\d{3})\])");
    std::smatch match;
    if (!std::regex_match(str, match, pattern)) {
        return TimeStampPair{0, 0};
    }
    auto ts1 = read_from_string(match[1]);
    auto ts2 = read_from_string(match[2]);
    if(!ts1 || !ts2) {
        return std::nullopt;
    }
    /// @todo this should to be optimized
    return TimeStampPair{ts1->milliseconds(), ts2->milliseconds()};
}

inline std::vector<TimeStampPair> read_timestamp_from_file(const std::string& file) {
    std::vector<TimeStampPair> result;
    std::ifstream ifs(file);
    std::string line;
    auto null = TimeStampPair{0, 0};
    while (std::getline(ifs, line)) {
        auto timestamp = read_pair_from_string(line);
        if (timestamp != std::nullopt) {
            result.push_back(timestamp.value());
        }
    }
    return result;
}

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