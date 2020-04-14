#pragma once

#include "ConnectionTracker.h"

#include <sys/select.h>

class DaviciInterface;

class MessageExchange
{
public:
    MessageExchange() {}
    ~MessageExchange() {}

    void SetDaviciInterface(DaviciInterface* pDaviciInterface);

    void Connect();
    void WaitForEvent();

private:
    int build_fd_sets(int fd, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);

    DaviciInterface* m_pDaviciInterface;
    ConnectionTracker m_connTracker;
};