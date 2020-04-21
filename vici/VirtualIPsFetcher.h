#pragma once

#include "SecurityAssociationList.h"

class DaviciInterface;

class VirtualIPsFetcher
{
public:
    VirtualIPsFetcher() : m_pDaviciInterface(0) {}
    ~VirtualIPsFetcher() {}

    void SetDaviciInterface(DaviciInterface* pDaviciInterface);
    void FetchSecurityAssociations();
    void ParseSecurityAssociationList();

private:
    DaviciInterface* m_pDaviciInterface;
    SecurityAssociationList m_saNewList;
    SecurityAssociationList m_saCompleteList;
};