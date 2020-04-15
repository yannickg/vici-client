#pragma once

#include <poll.h>

class ConnectionTracker
{
public:
    ConnectionTracker() :
        m_bConnectionReady(false)
    {

    }

    ~ConnectionTracker() {}

    enum fds {
        FD_VICI = 0,
        FD_TIMER = 1,
        FD_COUNT
    };

    struct pollfd m_pfd[FD_COUNT];
    bool m_bConnectionReady;
};