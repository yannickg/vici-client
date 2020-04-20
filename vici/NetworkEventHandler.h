#pragma once

#include "INetworkEventHandler.h"

class VirtualIPsFetcher;

class NetworkEventHandler : public INetworkEventHandler
{
public:
    NetworkEventHandler() :
        m_pVirtualIPsFetcher(0) {}
    ~NetworkEventHandler() {}

    void SetVirtualIPsFetcher(VirtualIPsFetcher* pVirtualIPsFetcher);

    virtual void onNetworkActivity();
    virtual void onFetchVIPsComplete();

private:
    VirtualIPsFetcher* m_pVirtualIPsFetcher;
};