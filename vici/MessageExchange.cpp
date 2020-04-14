#include "MessageExchange.h"

#include "DaviciInterface.h"

#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/poll.h>

#include <sys/eventfd.h>
#include <sys/epoll.h>

void MessageExchange::SetDaviciInterface(DaviciInterface* pDaviciInterface)
{
    m_pDaviciInterface = pDaviciInterface;
}

void MessageExchange::Connect()
{
    if (m_pDaviciInterface)
    {
        m_pDaviciInterface->Connect(m_connTracker);
    }
}

int MessageExchange::build_fd_sets(int fd, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
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

void MessageExchange::WaitForEvent()
{
    if (m_pDaviciInterface && m_connTracker.m_bConnectionReady)
    {
        std::cout << "DaviciInterface::WaitForEvent" << std::endl;

        if (m_connTracker.m_fd < 0)
        {
            std::cout << "file descriptor not ready" << std::endl;
            return;
        }

        struct pollfd pfds[1];
        pfds[0].fd = m_connTracker.m_fd;
        pfds[0].events = POLLIN;

        while (1)
        {
            std::cout << "poll()" << std::endl;
            int ret = poll(pfds, 1, -1);
            if (ret < 0)
            {
                return;
            }

            for (int i=0; i < 1; i++)
            {
                if (pfds[i].revents != 0)
                {
                    if (pfds[i].revents & POLLIN)
                    {
                        m_pDaviciInterface->ReadData();
                    }
                }
            }
        }

return;

        int ep_fd = epoll_create1(0);
        if (ep_fd < 0)
        {
            perror("epoll_create fail: ");
            return;
        }

        struct epoll_event events[64];
        struct epoll_event read_event;
        read_event.events = EPOLLIN | EPOLLET;//EPOLLHUP | EPOLLERR | EPOLLIN;
        read_event.data.fd = m_connTracker.m_fd;

        int ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, m_connTracker.m_fd, &read_event);
        if (ret < 0)
        {
            perror("epoll ctl failed:");
            return;
        }


        while (1)
        {
            ret = epoll_wait(ep_fd, events, 64, -1);
            std::cout << "Number of fd: " << ret << std::endl;
            if (ret > 0)
            {
                int i = 0;
                for (; i < ret; i++)
                {
                    std::cout << "Events: " << events[i].events << std::endl;

                    if ((events[i].events & EPOLLERR) ||
                        (events[i].events & EPOLLHUP) ||
                        (!(events[i].events & EPOLLIN)))
                    {
                        fprintf(stderr, "epoll error\n");
                        continue;
                    }
                    else if (m_connTracker.m_fd == events[i].data.fd)
                    {
                        m_pDaviciInterface->ReadData();
                    }


                    // if (events[i].events & EPOLLHUP)
                    // {
                    //     printf("epoll eventfd has epoll hup.\n");
                    //     break;
                    // }
                    // else if (events[i].events & EPOLLERR)
                    // {
                    //     printf("epoll eventfd has epoll error.\n");
                    //     break;
                    // }
                    // else if (events[i].events & EPOLLIN)
                    // {
                    //     uint64_t count = 0;
                    //     int event_fd = events[i].data.fd;
                    //     ret = read(event_fd, &count, sizeof(count));
                    //     if (ret < 0)
                    //     {
                    //         perror("read fail:");
                    //         break;
                    //     }
                    //     else
                    //     {
                    //     }
                    // }
                }
            }
            // else if (ret == 0)
            // {
            //     /* time out */
            //     printf("epoll wait timed out.\n");
            //     break;
            // }
            // else
            // {
            //     perror("epoll wait error:");
            //     break;
            // }
        }

        if (ep_fd >= 0)
        {
            close(ep_fd);
            ep_fd = -1;
        }

        return;












        /* Set nonblock for stdin. */
        // int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
        // flag |= O_NONBLOCK;
        // fcntl(STDIN_FILENO, F_SETFL, flag);

        fd_set read_fds;
        fd_set write_fds;
        fd_set except_fds;

        // server socket always will be greater then STDIN_FILENO
        int maxfd = m_connTracker.m_fd;

        while (1)
        {
            // Select() updates fd_set's, so we need to build fd_set's before each select()call.
            build_fd_sets(m_connTracker.m_fd, &read_fds, &write_fds, &except_fds);

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

                if (FD_ISSET(m_connTracker.m_fd, &read_fds))
                {
                    // std::cout << "FD_ISSET read_fds" << std::endl;
                    m_pDaviciInterface->ReadData();
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
    
}
