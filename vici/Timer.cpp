#include "Timer.h"
#include "TimerImpl.h"
#include <iostream>



std::map<unsigned int, TimerCallback> Timer::m_mapCallbacks;


Timer::Timer()
{
    TimerImpl::Instance();
}

Timer::~Timer()
{
}

unsigned int Timer::StartTimer(unsigned int interval, boost::function<void()> callback)
{
    std::cout << "Timer::StartTimer()" << std::endl;

    TimerCallback tmrCallback;
    tmrCallback.m_callback = callback;
    unsigned int nTimerId = TimerImpl::Instance()->start_timer(interval, handler, TimerImpl::TIMER_PERIODIC, (void*)this);
    m_mapCallbacks.insert(std::pair<unsigned int, TimerCallback>(nTimerId, tmrCallback));
    return nTimerId;
}

unsigned int Timer::CancelTimer(unsigned int nTimerId)
{
    TimerImpl::Instance()->stop_timer(nTimerId);
    m_mapCallbacks.erase(nTimerId);
    return 0;
}

void Timer::handler(size_t nTimerId, void * user_data)
{
    std::cout << "Timer::handler()" << std::endl;

    TimerImpl::Instance()->Signal(nTimerId);
}

void Timer::Callback(unsigned int nTimerId)
{
    std::cout << "Timer::Callback()" << std::endl;

    Timer::m_mapCallbacks[nTimerId].m_callback();
}

TimerMgr::TimerMgr() : m_fd(-1)
{
    TimerImpl::Instance();
}

TimerMgr::~TimerMgr()
{
    
}

int TimerMgr::ReadData()
{
    std::cout << "TimerMgr::ReadData()" << std::endl;

    if (m_fd == -1);
    {
        m_fd = GetWaitableObject();
    }

    uint64_t u = 0;
    ssize_t s = read(m_fd, &u, sizeof(uint64_t));

    unsigned int nTimerId = u;
    Timer::Callback(nTimerId);
}

int TimerMgr::GetWaitableObject()
{
    return TimerImpl::Instance()->GetWaitableObject();
}

