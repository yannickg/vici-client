#include "Timer.h"
#include "TimerImpl.h"


Timer::Timer()
{
    TimerImpl::Instance();
}

Timer::~Timer()
{
}

unsigned int Timer::StartTimer(unsigned int interval, boost::function<void()> callback)
{
    m_Callback =  callback;
    return TimerImpl::Instance()->start_timer(interval, handler, TimerImpl::TIMER_PERIODIC, (void*)this);
}

unsigned int Timer::CancelTimer(unsigned int nTimerId)
{

}

void Timer::handler(size_t timer_id, void * user_data)
{

}

TimerMgr::TimerMgr()
{
    TimerImpl::Instance();
}

TimerMgr::~TimerMgr()
{
    
}

int TimerMgr::GetEventFD()
{
    return TimerImpl::Instance()->GetEventFD();
}

