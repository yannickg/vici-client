#pragma once

#include <stdlib.h>

class TimerImpl
{
protected:
    TimerImpl() : m_thread_id(0), m_head(0) {}
public:
    ~TimerImpl() {}

    enum t_timer
    {
        TIMER_SINGLE_SHOT = 0, /*Periodic Timer*/
        TIMER_PERIODIC         /*Single Shot Timer*/
    };

    static TimerImpl* Instance();

    typedef void (*time_handler)(size_t timer_id, void * user_data);

    int initialize();
    size_t start_timer(unsigned int interval, time_handler handler, t_timer type, void * user_data);
    void stop_timer(size_t timer_id);
    void finalize();

    int GetEventFD();

private:
    struct timer_node
    {
        int                 fd;
        time_handler        callback;
        void*               user_data;
        unsigned int        interval;
        t_timer             type;
        timer_node*         next;
    };

    static TimerImpl* m_pSelf;

    timer_node* get_timer_from_fd(int fd);
    static void* timer_thread(void * data);
    void* MainLoop();

    pthread_t m_thread_id;
    timer_node* m_head;
};
