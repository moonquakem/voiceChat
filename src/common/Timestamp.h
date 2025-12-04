// ====================================================================
// LightVoice: Timestamp utility
// src/common/Timestamp.h
//
// A simple timestamp wrapper class for handling time-related
// operations. It's based on std::chrono for high precision.
//
// Author: Gemini
// ====================================================================

#pragma once

#include <chrono>
#include <string>

namespace lightvoice {

class Timestamp {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    // Get the current time as a Timestamp object
    static Timestamp now();

    // Convert to a formatted string (e.g., "YYYY-MM-DD HH:MM:SS.us")
    std::string toString() const;

    // Getters
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    // Constants
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

// Inline implementation for performance

inline Timestamp Timestamp::now() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

inline std::string Timestamp::toString() const {
    char buf[64] = {0};
    time_t seconds = secondsSinceEpoch();
    struct tm tm_time;

#ifdef _WIN32
    localtime_s(&tm_time, &seconds);
#else
    localtime_r(&seconds, &tm_time);
#endif

    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % Timestamp::kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);

    return buf;
}

} // namespace lightvoice
