#include "VirtualIPsFetcher.h"
#include "DaviciInterface.h"

void VirtualIPsFetcher::SetDaviciInterface(DaviciInterface* pDaviciInterface)
{
    m_pDaviciInterface = pDaviciInterface;
}

void VirtualIPsFetcher::FetchSecurityAssociations()
{
    if (m_pDaviciInterface)
    {
        m_pDaviciInterface->FetchSecurityAssociations();
    }
}
