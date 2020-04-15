#pragma once

#include "ConnectionTracker.h"

#include <sys/select.h>

class TimerMgr;
class DaviciInterface;

class MessageExchange
{
public:
    MessageExchange() {}
    ~MessageExchange() {}

    void SetTimerWaitableObject(TimerMgr& timerMgr);
    void SetDaviciInterface(DaviciInterface& daviciInterface);

    void Connect();
    void WaitForEvent();

private:
    DaviciInterface* m_pDaviciInterface;
    ConnectionTracker m_connTracker;
};