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

void NetworkEventHandler::onFetchVIPsTaskComplete()
{
    std::cout << "NetworkEventHandler::onFetchVIPsTaskComplete()" << std::endl;

    if (m_pVirtualIPsFetcher)
    {
        m_pVirtualIPsFetcher->ParseSecurityAssociationList();
    }
}