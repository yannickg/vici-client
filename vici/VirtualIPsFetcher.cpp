#include "VirtualIPsFetcher.h"
#include "DaviciInterface.h"

#include <iostream>

void VirtualIPsFetcher::SetDaviciInterface(DaviciInterface* pDaviciInterface)
{
    m_pDaviciInterface = pDaviciInterface;
}

void VirtualIPsFetcher::FetchSecurityAssociations()
{
    if (m_pDaviciInterface)
    {
        // TODO: create a vip list, pass it on
        m_pDaviciInterface->FetchSecurityAssociations(m_saList);
    }
}

void VirtualIPsFetcher::ParseSecurityAssociationList()
{
    std::cout << "VirtualIPsFetcher::ParseSecurityAssociationList()" << std::endl;

    std::map<std::string, SecurityAssociationItem>::iterator iter = m_saList.m_mapItems.begin();
    for (; iter != m_saList.m_mapItems.end(); ++iter)
    {
        std::cout << "***************************** m_remoteId:   " << iter->second.m_remoteId << std::endl;
        std::cout << "***************************** m_remoteHost: " << iter->second.m_remoteHost << std::endl;
        std::cout << "***************************** m_localId:    " << iter->second.m_localId << std::endl;
        std::cout << "***************************** m_localHost:  " << iter->second.m_localHost << std::endl;

        std::list<std::string>::iterator iterRVIPS = iter->second.m_remoteVips.begin();
        for (; iterRVIPS != iter->second.m_remoteVips.end(); ++iterRVIPS)
        {
            std::cout << "************************************* m_remoteVips: " << *iterRVIPS << std::endl;
        }
   }
}