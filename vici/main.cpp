#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <cstring>
#include <cassert>
#include <signal.h>
#include <pthread.h>

#include "DaviciInterface.h"
#include "NetworkEventHandler.h"
#include "VirtualIPsFetcher.h"
#include "MessageExchange.h"

static void
signalHandler(int signo)
{
    std::cout << std::endl << "Signal " << signo << " received" << std::endl;
    exit(-1);
}

int main(int argc, char* argv[])
{
    if ( signal( SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        std::cerr << "Couldn't install signal handler for SIGPIPE" << std::endl;
        exit(-1);
    }

    if ( signal( SIGINT, signalHandler ) == SIG_ERR )
    {
        std::cerr << "Couldn't install signal handler for SIGINT" << std::endl;
        exit( -1 );
    }

    if ( signal( SIGTERM, signalHandler ) == SIG_ERR )
    {
        std::cerr << "Couldn't install signal handler for SIGTERM" << std::endl;
        exit( -1 );
    }

    try
    {
        DaviciInterface davici;
        MessageExchange exchange;
        VirtualIPsFetcher vipsFetcher;
        NetworkEventHandler networkEventHandler;        

        exchange.SetDaviciInterface(&davici);
        vipsFetcher.SetDaviciInterface(&davici);
        davici.SetNetworkEventHandler(&networkEventHandler);
        networkEventHandler.SetVirtualIPsFetcher(&vipsFetcher);

        exchange.Connect();
        exchange.WaitForEvent();

        // pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
        // pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER;

        // pthread_mutex_lock(&ready_mutex);
        // while (1)
        // {
        //   pthread_cond_wait(&ready_cond, &ready_mutex);
        // }
        // pthread_mutex_unlock(&ready_mutex);
    }
    catch( ... )
    {
        std::cerr << "Caught execption" << std::endl;
        exit(-1);
    }

    return 0;
}
