#pragma once
#include <chrono>
#include <format>
#include <string>
#include <cstdint>
#include "QtCore/qstring.h"

struct TimeStamp {
    TimeStamp(int64_t milliseconds) : hh(milliseconds/3600000), mm((milliseconds%3600000)/60000)
                                    , ss((milliseconds%60000)/1000), ms(milliseconds%1000) {};
    std::string to_std_string();
    QString to_std_qstring();
    int64_t hh{0};
    int64_t mm{0};
    int64_t ss{0};
    int64_t ms{0};
};
inline std::string TimeStamp::to_std_string() {
    return std::format("{:02d}", hh) + std::format("{:02d}", mm) + std::format("{:02d}", ss) + std::format("{:03d}", ms);
}
inline QString TimeStamp::to_std_qstring() {
    return QString("%1:%2:%3,%4")
            .arg(hh, 2, 10, QChar('0'))
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'))
            .arg(ms, 3, 10, QChar('0'));
}

struct TimeStampPair {
    TimeStampPair(int64_t milliseconds1, int64_t milliseconds2) : timestamp1(milliseconds1), timestamp2(milliseconds2) {};
    TimeStamp timestamp1;
    TimeStamp timestamp2;
};

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