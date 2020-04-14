#pragma once

#include <boost/function.hpp>

class Timer
{
public:
    Timer();
    ~Timer();

    unsigned int StartTimer(unsigned int interval, boost::function<void()> callback);
    unsigned int CancelTimer(unsigned int nTimerId);

private:
    static void handler(size_t timer_id, void * user_data);

    boost::function<void()> m_Callback;
};

class TimerMgr
{
public:
    TimerMgr();
    ~TimerMgr();

    int GetEventFD();
};