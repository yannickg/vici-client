#include "TimerImpl.h"

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <poll.h>


#define MAX_TIMER_COUNT 1000

TimerImpl* TimerImpl::m_pSelf = NULL;


TimerImpl* TimerImpl::Instance()
{
    if (TimerImpl::m_pSelf == NULL)
    {
        TimerImpl::m_pSelf = new TimerImpl;
        TimerImpl::m_pSelf->initialize();
    }

    return TimerImpl::m_pSelf;
}

int TimerImpl::initialize()
{
    if (pthread_create(&m_thread_id, NULL, timer_thread, this))
    {
        /*Thread creation failed*/
        return 0;
    }

    return 1;
}

size_t TimerImpl::start_timer(unsigned int interval, time_handler handler, t_timer type, void * user_data)
{
    timer_node * new_node = new timer_node;
    if (new_node == NULL) 
    {
        return 0;
    }

    new_node->callback  = handler;
    new_node->user_data = user_data;
    new_node->interval  = interval;
    new_node->type      = type;

    new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

    if (new_node->fd == -1)
    {
        delete new_node;
        return 0;
    }

    struct itimerspec new_value;
    new_value.it_value.tv_sec = interval / 1000;
    new_value.it_value.tv_nsec = (interval % 1000)* 1000000;

    if (type == TIMER_PERIODIC)
    {
      new_value.it_interval.tv_sec= interval / 1000;
      new_value.it_interval.tv_nsec = (interval %1000) * 1000000;
    }
    else
    {
      new_value.it_interval.tv_sec= 0;
      new_value.it_interval.tv_nsec = 0;
    }

    timerfd_settime(new_node->fd, 0, &new_value, NULL);

    /*Inserting the timer node into the list*/
    new_node->next = m_head;
    m_head = new_node;

    return (size_t)new_node;
}

void TimerImpl::stop_timer(size_t timer_id)
{
    timer_node* tmp = NULL;
    timer_node* node = (timer_node*)timer_id;

    if (node == NULL)
    {
        return;
    }

    close(node->fd);

    if (node == m_head)
    {
        m_head = m_head->next;
    }
    else
    {

      tmp = m_head;

      while (tmp && tmp->next != node)
      {
        tmp = tmp->next;
      }

      if (tmp)
      {
          /*tmp->next can not be NULL here*/
          tmp->next = tmp->next->next;
      }
    }

    if (node)
    {
        delete node;
    }
}

void TimerImpl::finalize()
{
    while (m_head)
    {
        stop_timer((size_t)m_head);
    }

    pthread_cancel(m_thread_id);
    pthread_join(m_thread_id, NULL);
} 

TimerImpl::timer_node* TimerImpl::get_timer_from_fd(int fd)
{
    timer_node* tmp = m_head;

    while (tmp)
    {
        if (tmp->fd == fd)
        {
            return tmp;
        }

        tmp = tmp->next;
    }
    return NULL;
}

void* TimerImpl::timer_thread(void * data)
{
    TimerImpl* pSelf = static_cast<TimerImpl*>(data);
    if (pSelf)
    {
        return pSelf->MainLoop();
    }
}

void* TimerImpl::MainLoop()
{
    pollfd ufds[MAX_TIMER_COUNT] = {{0}};
    int iMaxCount = 0;
    timer_node * tmp = NULL;
    int read_fds = 0, i, s;
    uint64_t exp;

    while (1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        iMaxCount = 0;
        tmp = m_head;

        memset(ufds, 0, sizeof(struct pollfd)*MAX_TIMER_COUNT);
        while (tmp)
        {
            ufds[iMaxCount].fd = tmp->fd;
            ufds[iMaxCount].events = POLLIN;
            iMaxCount++;

            tmp = tmp->next;
        }
        read_fds = poll(ufds, iMaxCount, 100);

        if (read_fds <= 0)
        {
            continue;
        }

        for (i = 0; i < iMaxCount; i++)
        {
            if (ufds[i].revents & POLLIN)
            {
                s = read(ufds[i].fd, &exp, sizeof(uint64_t));

                if (s != sizeof(uint64_t))
                {
                    continue;
                }

                tmp = get_timer_from_fd(ufds[i].fd);

                if (tmp && tmp->callback)
                {
                    tmp->callback((size_t)tmp, tmp->user_data);
                }
            }
        }
    }        

    return NULL;
}

int TimerImpl::GetEventFD()
{

}

