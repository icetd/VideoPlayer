#include "MTimer.h"
#include "log.h"

MTimer::MTimer(uint32_t interval_msec) :
    m_interval_msec(interval_msec)
{
}

MTimer::~MTimer()
{
}

void MTimer::Start()
{
    this->start();
    this->detach();
}

void MTimer::SetInterval(uint32_t msec)
{
    m_interval_msec = msec;
}

void MTimer::SetTimerCallback(std::function<void(void)> callback)
{
    OnTimerCallback = std::move(callback);
}

void MTimer::run()
{
    while(!this->isStoped()) {
        this->sleepMs(m_interval_msec);
        try {
            OnTimerCallback();
        } catch (const std::exception &ex) {
            LOG(ERRO, "Exception in timer callback: %s", ex.what());
        }
    }
}