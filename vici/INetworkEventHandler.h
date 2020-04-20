#pragma once

class INetworkEventHandler
{
protected:
    INetworkEventHandler() :
        m_bReady(false) {}
    ~INetworkEventHandler() {}

    bool m_bReady;

public:
    void SetReady(bool bReady)
    {
        m_bReady = bReady;
    }
    bool IsReady() const
    {
        return m_bReady;
    }

    virtual void onNetworkActivity() = 0;
    virtual void onFetchVIPsComplete() = 0;
};