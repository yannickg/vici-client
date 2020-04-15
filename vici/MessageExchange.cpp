#include "MessageExchange.h"

#include "DaviciInterface.h"
#include "Timer.h"

#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/poll.h>

#include <sys/eventfd.h>

void MessageExchange::SetTimerWaitableObject(TimerMgr& timerMgr)
{
    std::cout << "DaviciInterface::SetTimerWaitableObject" << std::endl;

    m_connTracker.m_pfd[ConnectionTracker::FD_TIMER].fd = timerMgr.GetWaitableObject();
    m_connTracker.m_pfd[ConnectionTracker::FD_TIMER].events = POLLIN;
}

void MessageExchange::SetDaviciInterface(DaviciInterface& daviciInterface)
{
    std::cout << "DaviciInterface::SetDaviciInterface" << std::endl;

    m_pDaviciInterface = &daviciInterface;
}

void MessageExchange::Connect()
{
    std::cout << "DaviciInterface::Connect" << std::endl;

    if (m_pDaviciInterface)
    {
        m_pDaviciInterface->Connect(m_connTracker);
    }
}

void MessageExchange::WaitForEvent()
{
    std::cout << "DaviciInterface::WaitForEvent" << std::endl;

    if (m_pDaviciInterface && m_connTracker.m_bConnectionReady)
    {
        while (1)
        {
            int size = sizeof(m_connTracker.m_pfd) / sizeof(m_connTracker.m_pfd[0]);
            int ret = poll(m_connTracker.m_pfd, size, -1);
            if (ret < 0)
            {
                std::cout << "poll() return value is < 0" << std::endl;
                return;
            }

            for (int i=0; i < size; i++)
            {
                if (m_connTracker.m_pfd[i].revents != 0)
                {
                    if (m_connTracker.m_pfd[i].fd == m_connTracker.m_pfd[ConnectionTracker::FD_VICI].fd)
                    {
                        if (m_connTracker.m_pfd[i].revents & POLLIN)
                        {
                            m_pDaviciInterface->ReadData();
                        }
                        else if (m_connTracker.m_pfd[i].revents & POLLOUT)
                        {
                            m_pDaviciInterface->WriteData();
                        }
                    }
                    else if (m_connTracker.m_pfd[i].fd == m_connTracker.m_pfd[ConnectionTracker::FD_TIMER].fd)
                    {
                        if (m_connTracker.m_pfd[i].revents & POLLIN)
                        {
                std::cout << "yahoo!" << std::endl;
                            TimerMgr tmrMgr;
                            tmrMgr.ReadData();
                        }
                        else if (m_connTracker.m_pfd[i].revents & POLLOUT)
                        {
                        }
                    }
                }
            }
        }
    }
}
