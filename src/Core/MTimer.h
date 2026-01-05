#ifndef TIMER_H
#define TIMER_H

#include "MThread.h"
#include <functional>

class MTimer : public MThread
{
public:
    MTimer(uint32_t interval_msec);
    ~MTimer();
    void Start();
    void SetInterval(uint32_t msec);
    void SetTimerCallback(std::function<void (void)> callback);

    virtual void run() override;
private:

    std::function<void (void)> OnTimerCallback;
    uint32_t m_interval_msec;
};

#endif