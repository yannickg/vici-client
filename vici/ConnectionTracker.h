#pragma once

class ConnectionTracker
{
public:
    ConnectionTracker() : m_fd(-1), m_bConnectionReady(false) {}
    ~ConnectionTracker() {}

    int m_fd;
    bool m_bConnectionReady;
};