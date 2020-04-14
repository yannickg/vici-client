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
    m_pNetworkEventHandler(0), m_conn(0), m_fd(0), m_bConnectionReady(false), m_nTimerId(0), m_pConnTracker(0)
{
}

DaviciInterface::~DaviciInterface()
{

}

int DaviciInterface::vici_fdcallback(struct davici_conn *conn, int fd, int ops, void *user)
{
    std::cout << "DaviciInterface::vici_fdcallback()" << std::endl;

    ConnectionTracker* pConnTracker = static_cast<ConnectionTracker*>(user);
    if (pConnTracker)
    {
        std::cout << "Ready!!! - fd: " << fd << std::endl;
        pConnTracker->m_fd = fd;
        pConnTracker->m_bConnectionReady = true;
    }

    if (ops & DAVICI_READ)
    {
        std::cout << "vici_fdcallback - ops: DAVICI_READ" << std::endl;
    }
    if (ops & DAVICI_WRITE)
    {
        std::cout << "vici_fdcallback - ops: DAVICI_WRITE" << std::endl;
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
        Timer timer;
        if (pSelf->m_nTimerId == 0)
        {
            pSelf->m_nTimerId = timer.CancelTimer(pSelf->m_nTimerId);
        }

        pSelf->m_nTimerId = timer.StartTimer(1000, boost::bind(&DaviciInterface::onNetworkActivity, pSelf));
        
        // pSelf->m_pNetworkEventHandler->onNetworkActivity();
    }
}

void DaviciInterface::onNetworkActivity()
{
    std::cout << "DaviciInterface::onNetworkActivity()" << std::endl;
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

int DaviciInterface::build_fd_sets(int fd, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
    FD_ZERO(read_fds);
    //FD_SET(STDIN_FILENO, read_fds);
    FD_SET(fd, read_fds);

    FD_ZERO(write_fds);
    // there is smth to send, set up write_fd for server socket
    //if (server->send_buffer.current > 0)
    //FD_SET(fd, write_fds);

    FD_ZERO(except_fds);
    //FD_SET(STDIN_FILENO, except_fds);
    FD_SET(fd, except_fds);

    return 0;
}

void DaviciInterface::Connect(ConnectionTracker& connTracker)
{
    std::cout << "DaviciInterface::Connect()" << std::endl;

    m_pConnTracker = &connTracker;

    assert(davici_connect_unix(vici_socket, vici_fdcallback, &connTracker, &m_conn) == 0);
    
    assert(davici_register(m_conn, "log", eventlog, this) >= 0);
    assert(davici_register(m_conn, "list-sa", eventlistsa, this) >= 0);

    davici_write(m_conn);
}

void DaviciInterface::ReadData()
{
    davici_read(m_conn);
}

void DaviciInterface::FetchSecurityAssociations()
{
    std::cout << "DaviciInterface::FetchSecurityAssociations()" << std::endl;

    struct davici_request *reqp;
    davici_new_cmd("list-sas", &reqp);
    davici_queue(m_conn, reqp, eventlistsas, this);
    davici_write(m_conn);
}

void DaviciInterface::WaitForEvent()
{
    std::cout << "DaviciInterface::WaitForEvent" << std::endl;

    /* Set nonblock for stdin. */
    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    // server socket always will be greater then STDIN_FILENO
    int maxfd = m_fd;

    while (1)
    {
        // Select() updates fd_set's, so we need to build fd_set's before each select()call.
        build_fd_sets(m_fd, &read_fds, &write_fds, &except_fds);

        int activity = select(maxfd + 1, &read_fds, NULL, NULL, NULL);

        switch (activity)
        {
            case -1:
            printf("select() returns -1.\n");
            perror("select()");
            exit(-1);
            //shutdown_properly(EXIT_FAILURE);

            case 0:
            // you should never get here
            printf("select() returns 0.\n");
            exit(-1);
            //shutdown_properly(EXIT_FAILURE);

            default:
            /* All fd_set's should be checked. */
            //if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            //if (handle_read_from_stdin(&server, client_name) != 0)
            //shutdown_properly(EXIT_FAILURE);
            //}

            //if (FD_ISSET(STDIN_FILENO, &except_fds)) {
            //printf("except_fds for stdin.\n");
            //shutdown_properly(EXIT_FAILURE);
            //}

            if (FD_ISSET(m_fd, &read_fds))
            {
                // std::cout << "FD_ISSET read_fds" << std::endl;
                m_peer.Reinit();
                davici_read(m_conn);
                //   if (Receive() != 0)
                //   {
                //       std::cout << "Receive() returned non zero value" << std::endl;
                //       return;
                //   }
            }

            // if (FD_ISSET(m_fd, &write_fds)) {
            //   std::cout << "FD_ISSET write_fds" << std::endl;
            //   //if (send_to_peer(&server) != 0)
            //     //shutdown_properly(EXIT_FAILURE);
            // }

            // if (FD_ISSET(m_fd, &except_fds)) {
            //   printf("except_fds for server.\n");
            //   exit(-1);
            //   //shutdown_properly(EXIT_FAILURE);
            // }
        }
    }
}

/* Receive message from peer and handle it with message_handler(). */
int DaviciInterface::Receive()
{
#if 0
  printf("Ready for recv()\n");
  
  size_t len_to_receive;
  ssize_t received_count;
  size_t received_total = 0;
  do {
    // Is completely received?
    if (m_peer.current_receiving_byte >= sizeof(m_peer.receiving_buffer))
    {
      m_peer.current_receiving_byte = 0;
    }
    
    // Count bytes to send.
    len_to_receive = sizeof(m_peer.receiving_buffer) - m_peer.current_receiving_byte;
    if (len_to_receive > MAX_SEND_SIZE)
    {
      len_to_receive = MAX_SEND_SIZE;
    }
    
    printf("Let's try to recv() %zd bytes... \n", len_to_receive);
    received_count = recv(m_fd, m_peer.receiving_buffer.m_data + m_peer.current_receiving_byte, len_to_receive, MSG_DONTWAIT);
    if (received_count < 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        printf("peer is not ready right now, try again later.\n");
        break;
      }
      else
      {
        std::cout << "errno: " << errno << std::endl;
        perror("recv() from peer error\n");
        return -1;
      }
    } 
    // If recv() returns 0, it means that peer gracefully shutdown. Shutdown client.
    else if (received_count == 0)
    {
      printf("recv() 0 bytes. Peer gracefully shutdown.\n");
      return -1;
    }
    else if (received_count > 0)
    {
      m_peer.current_receiving_byte += received_count;
      received_total += received_count;
      printf("recv() %zd bytes\n", received_count);
    }

  } while (received_count > 0);
  
    printf("Total recv()'ed %zu bytes.\n", received_total);
    printf("bytes: %s\n\n", m_peer.receiving_buffer.m_data);
#endif
    return 0;
}

void DaviciInterface::SetNetworkEventHandler(INetworkEventHandler* pNetworkEventHandler)
{
    m_pNetworkEventHandler = pNetworkEventHandler;
}

bool DaviciInterface::ConnectionIsReady() const
{
    return m_bConnectionReady;
}
