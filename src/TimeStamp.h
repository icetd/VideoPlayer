#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <string>

class TimeStamp
{
public:
    TimeStamp() : m_micro_seconds_since_epoch(0)
    {
    }

    explicit TimeStamp(int64_t microSecondsSinceEpoch) : m_micro_seconds_since_epoch(microSecondsSinceEpoch)
    {
    }

    static TimeStamp now();
    static TimeStamp invalid() { return TimeStamp(); }

    bool valid() const { return m_micro_seconds_since_epoch > 0; }

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;
    int64_t microSecondsSinceEpoch() const { return m_micro_seconds_since_epoch; }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t m_micro_seconds_since_epoch;
};

inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline double timeDifference(TimeStamp high, TimeStamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff);
}

inline TimeStamp addTime(TimeStamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimeStamp::kMicroSecondsPerSecond);
    return TimeStamp(timestamp.microSecondsSinceEpoch() + delta);
}

#endif