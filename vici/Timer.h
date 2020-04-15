#pragma once

#include <boost/function.hpp>
#include <map>

class TimerCallback
{
public:
    boost::function<void()> m_callback;
};

class Timer
{
public:
    Timer();
    ~Timer();

    unsigned int StartTimer(unsigned int interval, boost::function<void()> callback);
    unsigned int CancelTimer(unsigned int nTimerId);

    static void Callback(unsigned int nTimerId);

private:
    static void handler(size_t nTimerId, void * user_data);

    static std::map<unsigned int, TimerCallback> m_mapCallbacks;
};

class TimerMgr
{
public:
    TimerMgr();
    ~TimerMgr();

    int ReadData();
    int GetWaitableObject();

private:
    int m_fd;
};