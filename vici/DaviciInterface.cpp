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
    m_pNetworkEventHandler(0), m_conn(0), m_fd(0), m_nTimerId(0), m_pConnTracker(0), m_pSAList(0)
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
    if (pSelf)
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

    if (m_pNetworkEventHandler)
    {
        m_pNetworkEventHandler->onNetworkActivity();
    }
}


void DaviciInterface::eventlistsas(struct davici_conn *c, int err, const char *name,
					struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::eventlistsas()" << std::endl;

    // TODO: this is where the command gets completed
    DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);
    if (pSelf && pSelf->m_pNetworkEventHandler)
    {
        pSelf->m_pNetworkEventHandler->onFetchVIPsTaskComplete();
    }
}


int DaviciInterface::section(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::section()" << std::endl;

    std::cout << "***Level: " << davici_get_level(res) << std::endl;

    int recurse, parse;
    do 
    {
        
        do 
        {
            recurse = davici_recurse(res, section, list, keyvalue, user);
            std::cout << "[section] - recurse's return value: " << recurse << std::endl;
        }
        while (recurse > 0);

        parse = davici_parse(res);
        if (parse == DAVICI_LIST_ITEM)
        {
            std::cout << "[section] - found a list inside section, invoking list() now" << std::endl;
            parse = list(res, user);
        }
        std::cout << "[section] - parse's return value: " << parse << std::endl;
    }
    while (parse != 0 && recurse != 0);

    return 0;
}


int DaviciInterface::list(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::list()" << std::endl;

    SecurityAssociationItem* pSAItem = static_cast<SecurityAssociationItem*>(user);
    if (pSAItem)
    {
        int element = -1;// = davici_parse(res);

        do
        {
            std::cout << "***Level: " << davici_get_level(res) << std::endl;
            if (davici_name_strcmp(res, "remote-vips") == 0)
            {
                char value[32];
                davici_get_value_str(res, value, sizeof(value));
                pSAItem->m_remoteVips.push_back(value);
            }
        }
        while ((element = davici_parse(res)) != DAVICI_LIST_END);
    }

    return 0;
}


int DaviciInterface::keyvalue(struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::keyvalue()" << std::endl;

    SecurityAssociationItem* pSAItem = static_cast<SecurityAssociationItem*>(user);
    if (pSAItem)
    {
        std::cout << "***Level: " << davici_get_level(res) << std::endl;

        if (davici_name_strcmp(res, "local-host") == 0)
        {
            char value[32];
            davici_get_value_str(res, value, sizeof(value));
            pSAItem->m_localHost = value;
        }
        else if (davici_name_strcmp(res, "local-id") == 0)
        {
            char value[32];
            davici_get_value_str(res, value, sizeof(value));
            pSAItem->m_localId = value;
        }
        else if (davici_name_strcmp(res, "remote-host") == 0)
        {
            char value[32];
            davici_get_value_str(res, value, sizeof(value));
            pSAItem->m_remoteHost = value;
        }
        else if (davici_name_strcmp(res, "remote-id") == 0)
        {
            char value[32];
            davici_get_value_str(res, value, sizeof(value));
            pSAItem->m_remoteId = value;
        }
    }

    return 0;
}


void DaviciInterface::eventlistsa(struct davici_conn *c, int err, const char *name,
					struct davici_response *res, void *user)
{
    std::cout << "DaviciInterface::eventlistsa()" << std::endl;

    assert(err >= 0);
    if (res)
    {
        DaviciInterface* pSelf = static_cast<DaviciInterface*>(user);
        if (pSelf && pSelf->m_pSAList)
        {
            std::cout << "***Level: " << davici_get_level(res) << std::endl;

            int recurse, parse;
            SecurityAssociationItem saItem;
            do 
            {                
                do 
                {
                    recurse = davici_recurse(res, section, list, keyvalue, &saItem);
                    std::cout << "recurse's return value: " << recurse << std::endl;
                }
                while (recurse > 0);

                parse = davici_parse(res);
                if (parse == DAVICI_LIST_ITEM)
                {
                    std::cout << "found a list inside key/value pair, invoking list() now" << std::endl;
                    parse = list(res, &saItem);
                }
                std::cout << "  parse's return value: " << parse << std::endl;
            }
            while (parse != 0 && recurse != 0);

            pSelf->m_pSAList->Add(saItem);
        }
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
    
    std::cout << "DaviciInterface::eventlistsa() OUT" << std::endl;
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

void DaviciInterface::FetchSecurityAssociations(SecurityAssociationList& saList)
{
    std::cout << "DaviciInterface::FetchSecurityAssociations()" << std::endl;

    // TODO: receive vip list object, store it
    m_pSAList = &saList;

    struct davici_request *reqp;
    davici_new_cmd("list-sas", &reqp);
    davici_queue(m_conn, reqp, eventlistsas, this);

    std::cout << "DaviciInterface::FetchSecurityAssociations() - OUT" << std::endl;
}

void DaviciInterface::SetNetworkEventHandler(INetworkEventHandler* pNetworkEventHandler)
{
    m_pNetworkEventHandler = pNetworkEventHandler;
}
