#pragma once

#include "SecurityAssociationList.h"

#include <stdio.h>
#include <memory.h>

#include <sys/select.h>

class INetworkEventHandler;
class ConnectionTracker;

/* Maximum bytes that can be send() or recv() via net by one call.
 * It's a good idea to test sending one byte by one.
 */
#define MAX_SEND_SIZE 100

/* Size of send queue (messages). */
#define MAX_MESSAGES_BUFFER_SIZE 10

#define SENDER_MAXSIZE 128
#define DATA_MAXSIZE 512

class DaviciInterface
{
public:
    DaviciInterface();
    ~DaviciInterface();

    void Connect(ConnectionTracker& connTracker);

    void ReadData();
    void WriteData();

    void FetchSecurityAssociations(SecurityAssociationList& saList);

    bool ConnectionIsReady() const;
    void SetNetworkEventHandler(INetworkEventHandler* pNetworkEventHandler);

private:
    static int vici_fdcallback(struct davici_conn *conn, int fd, int ops, void *user);
    static void eventlog(struct davici_conn *c, int err, const char *name,
				    	struct davici_response *res, void *user);
    static void eventlistsas(struct davici_conn *c, int err, const char *name,
                            struct davici_response *res, void *user);
                            
    static int section(struct davici_response *res, void *user);
    static int list(struct davici_response *res, void *user);
    static int keyvalue(struct davici_response *res, void *user);
    static void eventlistsa(struct davici_conn *c, int err, const char *name,
                            struct davici_response *res, void *user);

    void onEventlog(struct davici_conn *c, int err, struct davici_response *res);
    void onNetworkActivity();

    class Message
    {
    public:
        Message()
        {
            Reinit();
        }
        ~Message() {}

        void Reinit()
        {
            memset(m_data, 0, sizeof(m_data));
        }

        char m_data[DATA_MAXSIZE];
    };

    class MessageQueue
    {
    public:
        MessageQueue() : m_pData(0), m_size(0), m_current(0) {}
        ~MessageQueue()
        {
            if (m_pData)
            {
                delete [] m_pData;
            }
        }

        void Create(int size)
        {
            m_pData = new Message[size];
            m_size = size;
            m_current = 0;
        }

        void Reinit()
        {
            if (m_pData)
            {
                m_pData->Reinit();
                m_current = 0;
            }
        }

        Message* m_pData;
        int m_size;
        int m_current;
    };

    class Peer
    {
    public:
        Peer() :
            current_sending_byte(0), current_receiving_byte(0)
        {
            send_buffer.Create(MAX_MESSAGES_BUFFER_SIZE);
        }

        ~Peer() {}

        void Reinit()
        {
            send_buffer.Reinit();
            sending_buffer.Reinit();
            current_sending_byte = 0;
            receiving_buffer.Reinit();
            current_receiving_byte = 0;
        }

        /* Messages that waiting for send. */
        MessageQueue send_buffer;
        
        /* Buffered sending message.
        * 
        * In case we doesn't send whole message per one call send().
        * And current_sending_byte is a pointer to the part of data that will be send next call.
        */
        Message sending_buffer;
        size_t current_sending_byte;
        
        /* The same for the receiving message. */
        Message receiving_buffer;
        size_t current_receiving_byte;
    };

    INetworkEventHandler* m_pNetworkEventHandler;
    ConnectionTracker* m_pConnTracker;
    SecurityAssociationList* m_pSAList;

    Peer m_peer;
    int m_fd;
	davici_conn* m_conn;
    unsigned int m_nTimerId;
};
