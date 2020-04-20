#include "NetworkEventHandler.h"
#include "VirtualIPsFetcher.h"

#include <iostream>

void NetworkEventHandler::SetVirtualIPsFetcher(VirtualIPsFetcher* pVirtualIPsFetcher)
{
    m_pVirtualIPsFetcher = pVirtualIPsFetcher;
}

void NetworkEventHandler::onNetworkActivity()
{
    std::cout << "NetworkEventHandler::onNetworkActivity()" << std::endl;

    if (m_pVirtualIPsFetcher)
    {
        m_pVirtualIPsFetcher->FetchSecurityAssociations();
    }
}

void NetworkEventHandler::onFetchVIPsComplete()
{
    std::cout << "NetworkEventHandler::onFetchVIPsComplete()" << std::endl;

    if (m_pVirtualIPsFetcher)
    {
        m_pVirtualIPsFetcher->ParseSecurityAssociationList();
    }
}