#include "DaviciInterface.h"
#include "INetworkEventHandler.h"
#include "ConnectionTracker.h"
#include "Timer.h"

#include <stdlib.h>
#include <iostream>
#include <cassert>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <davici.h>
#include <boost/bind.hpp>



static const char vici_socket[] = "/var/run/charon.vici";


DaviciInterface::DaviciInterface() :
    m_pNetworkEventHandler(0), m_conn(0), m_fd(0), m_nTimerId(0), m_pConnTracker(0)
{
}

DaviciInterface::~DaviciInterface()
{

}

int DaviciInterface::vici_fdcallback(struct davici_conn *conn, int fd, int ops, void *user)
{
    std::cout << "DaviciInterface::vici_fdcallback() - fd: " << fd << std::endl;

    ConnectionTracker* pConnTracker = static_cast<ConnectionTracker*>(user);
    if (pConnTracker)
    {
        pConnTracker->m_bConnectionReady = true;
        pConnTracker->m_pfd[ConnectionTracker::FD_VICI].fd = fd;
        pConnTracker->m_pfd[ConnectionTracker::FD_VICI].events = 0;

        if (ops & DAVICI_READ)
        {
            std::cout << "vici_fdcallback - ops: DAVICI_READ" << std::endl;
            pConnTracker->m_pfd[ConnectionTracker::FD_VICI].events |= POLLIN;
        }
        if (ops & DAVICI_WRITE)
        {
            std::cout << "vici_fdcallback - ops: DAVICI_WRITE" << std::endl;
            pConnTracker->m_pfd[ConnectionTracker::FD_VICI].events |= POLLOUT;
        }
    }

    return 0;
}


void DaviciInterface::eventlog(struct davici_conn *c, int err, const char *name,
					struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::eventlog()" << std::endl;

    DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);
    if (pSelf && pSelf->m_pNetworkEventHandler && pSelf->m_pNetworkEventHandler->IsReady())
    {
        pSelf->onEventlog(c, err, res);
    }
}

void DaviciInterface::onEventlog(struct davici_conn *c, int err, struct davici_response *res)
{
    std::cout << "DaviciInterface::onEventlog()" << std::endl;

    if (m_pNetworkEventHandler && m_pNetworkEventHandler->IsReady())
    {
        std::cout << "timerid: " << m_nTimerId << std::endl;

        Timer timer;
        if (m_nTimerId != 0)
        {
            m_nTimerId = timer.CancelTimer(m_nTimerId);
        }

        m_nTimerId = timer.StartTimer(1000, boost::bind(&DaviciInterface::onNetworkActivity, this));
    }
}

void DaviciInterface::onNetworkActivity()
{
    std::cout << "DaviciInterface::onNetworkActivity()" << std::endl;

    Timer timer;
    m_nTimerId = timer.CancelTimer(m_nTimerId);

    m_pNetworkEventHandler->onNetworkActivity();
}


void DaviciInterface::eventlistsas(struct davici_conn *c, int err, const char *name,
					struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::eventlistsas()" << std::endl;

    DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);
    if (pSelf && pSelf->m_pNetworkEventHandler)
    {
    }

    assert(err >= 0);
    if (res)
    {
        int element = davici_parse(res);
        // std::cout << "element: " << element << std::endl;
    }
}


int DaviciInterface::section(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::section()" << std::endl;

    int element = davici_parse(res);
    // std::cout << "element: " << element << std::endl;

    const char* name = davici_get_name(res);
    // std::cout << "name: " << name << std::endl;

    char value[32];
    int ret1 = davici_get_value_str(res, value, sizeof(value));
    // std::cout << "value: " << value << std::endl;

    int ret2 = davici_recurse(res, section, list, keyvalue, NULL);

    return 0;
}


int DaviciInterface::list(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::list()" << std::endl;

    int type = davici_parse(res);

    std::cout << "##################################################" << std::endl;
    do
    {
        std::cout << "element: " << type << std::endl;
        const char* name = davici_get_name(res);
        std::cout << "name: " << name << std::endl;

        char value[32];
        int ret1 = davici_get_value_str(res, value, sizeof(value));
        std::cout << "value: " << value << std::endl;
    }
    while ((type = davici_parse(res)) != DAVICI_LIST_END);

    std::cout << "##################################################" << std::endl;

    return 0;
}


int DaviciInterface::keyvalue(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::keyvalue()" << std::endl;

    int element = davici_parse(res);
    // std::cout << "element: " << element << std::endl;

    if (element == DAVICI_LIST_START)
    {
        // std::cout << "found a list inside key/value pair, invoke list() now" << std::endl;
        return list(res, user);
    }
    else
    {
        const char* name = davici_get_name(res);
        // std::cout << "name: " << name << std::endl;

        char value[32];
        int ret1 = davici_get_value_str(res, value, sizeof(value));
        // std::cout << "value: " << value << std::endl;    
    }

    return 0;
}


void DaviciInterface::eventlistsa(struct davici_conn *c, int err, const char *name,
					struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::eventlistsa()" << std::endl;

    DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);

    assert(err >= 0);
    if (res)
    {
        int element = davici_parse(res); // 1 (DAVICI_SECTION_START)
        // std::cout << "element: " << element << std::endl;

        const char* name = davici_get_name(res);
        // std::cout << "name: " << name << std::endl;

        char value[32];
        int ret1 = davici_get_value_str(res, value, sizeof(value));
        // std::cout << "value: " << value << std::endl;

        int ret2 = davici_recurse(res, section, list, keyvalue, NULL);
    }
    else
    {
        DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);
        if (pSelf && pSelf->m_pNetworkEventHandler)
        {
            std::cout << "Interface ready!" << std::endl;
            pSelf->m_pNetworkEventHandler->SetReady(true);
        }
    }
}

void DaviciInterface::Connect(ConnectionTracker& connTracker)
{
    std::cout << "DaviciInterface::Connect()" << std::endl;

    m_pConnTracker = &connTracker;

    assert(davici_connect_unix(vici_socket, vici_fdcallback, &connTracker, &m_conn) == 0);
    
    assert(davici_register(m_conn, "log", eventlog, this) >= 0);
    assert(davici_register(m_conn, "list-sa", eventlistsa, this) >= 0);
}

void DaviciInterface::ReadData()
{
    std::cout << "DaviciInterface::ReadData()" << std::endl;
    davici_read(m_conn);
}

void DaviciInterface::WriteData()
{
    std::cout << "DaviciInterface::WriteData()" << std::endl;
    davici_write(m_conn);
}

void DaviciInterface::FetchSecurityAssociations()
{
    std::cout << "DaviciInterface::FetchSecurityAssociations()" << std::endl;

    struct davici_request *reqp;
    davici_new_cmd("list-sas", &reqp);
    davici_queue(m_conn, reqp, eventlistsas, this);
}

void DaviciInterface::SetNetworkEventHandler(INetworkEventHandler* pNetworkEventHandler)
{
    m_pNetworkEventHandler = pNetworkEventHandler;
}
