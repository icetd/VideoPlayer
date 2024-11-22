#include "TimeStamp.h"
#include <chrono>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#endif

static_assert(sizeof(TimeStamp) == sizeof(int64_t),
    "TimeStamp should be same as int64_t");

std::string TimeStamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_micro_seconds_since_epoch / kMicroSecondsPerSecond;
    int64_t microseconds = m_micro_seconds_since_epoch % kMicroSecondsPerSecond;
    return buf;
}

std::string TimeStamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(m_micro_seconds_since_epoch / kMicroSecondsPerSecond);
    struct tm tm_time;
#ifdef _WIN32
    gmtime_s(&tm_time, &seconds);
#else
    gmtime_r(&seconds, &tm_time); // POSIX 系统支持 gmtime_r
#endif

    // 东八区偏移量：UTC+8，即 8小时 = 8 * 3600秒 = 28800秒
    int timezoneOffsetSeconds = 8 * 3600;
    
    // 手动调整时区
    tm_time.tm_hour += timezoneOffsetSeconds / 3600; // 加上小时偏移
    tm_time.tm_min += (timezoneOffsetSeconds % 3600) / 60; // 加上分钟偏移

    // 处理溢出，确保时间在合理范围内
    if (tm_time.tm_min >= 60) {
        tm_time.tm_hour += tm_time.tm_min / 60;
        tm_time.tm_min %= 60;
    }

    if (tm_time.tm_hour >= 24) {
        tm_time.tm_hour %= 24;
    }

    if (showMicroseconds) {
        int microseconds = static_cast<int>(m_micro_seconds_since_epoch % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d-%02d-%02d-%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

TimeStamp TimeStamp::now()
{
    auto counts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    return TimeStamp(counts);
}