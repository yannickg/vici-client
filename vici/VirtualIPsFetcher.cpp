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
        m_pDaviciInterface->FetchSecurityAssociations(m_saNewList);
    }
}

void VirtualIPsFetcher::ParseSecurityAssociationList()
{
    std::cout << "VirtualIPsFetcher::ParseSecurityAssociationList()" << std::endl;

    SecurityAssociationList saOrigList;
    SecurityAssociationList saNewList;

    SecurityAssociationList saAddedList;
    SecurityAssociationList saDeletedList;
    SecurityAssociationList saChangedList;

    saOrigList.Copy(m_saCompleteList);
    saNewList.Copy(m_saNewList);
    saDeletedList.Copy(saOrigList);

    std::cout << "m_saCompleteList: " << m_saCompleteList.Size() << std::endl;
    std::cout << "m_saNewList: " << m_saNewList.Size() << std::endl;
    std::cout << "saOrigList: " << saOrigList.Size() << std::endl;
    std::cout << "saNewList: " << saNewList.Size() << std::endl;
    std::cout << "saAddedList: " << saAddedList.Size() << std::endl;
    std::cout << "saDeletedList: " << saDeletedList.Size() << std::endl;
    std::cout << "saChangedList: " << saChangedList.Size() << std::endl;

    std::map<std::string, SecurityAssociationItem>::iterator iter1 = saNewList.m_mapItems.begin();
    for (; iter1 != saNewList.m_mapItems.end(); ++iter1)
    {
        std::cout << "Looking for item: " << iter1->first << std::endl;
        std::map<std::string, SecurityAssociationItem>::iterator iter2 = saDeletedList.m_mapItems.find(iter1->first);
        if (iter2 == saDeletedList.m_mapItems.end())
        {
            std::cout << "Item not found, adding to new list" << std::endl;
            saAddedList.Add(iter1->second);
        }
        else
        {
            if (iter1->second.AnythingHasChanged(iter2->second))
            {
                std::cout << "Item has changed, adding to changed list" << std::endl;
                saChangedList.Add(iter1->second);
            }

            std::cout << "Deleting item" << std::endl;
            saDeletedList.m_mapItems.erase(iter2);
        }        
    }

    std::cout << "Added list" << std::endl;
    std::map<std::string, SecurityAssociationItem>::iterator iter3 = saAddedList.m_mapItems.begin();
    for (; iter3 != saAddedList.m_mapItems.end(); ++iter3)
    {
        std::cout << "***************************** m_remoteId:   " << iter3->second.m_remoteId << std::endl;
        std::cout << "***************************** m_remoteHost: " << iter3->second.m_remoteHost << std::endl;
        std::cout << "***************************** m_localId:    " << iter3->second.m_localId << std::endl;
        std::cout << "***************************** m_localHost:  " << iter3->second.m_localHost << std::endl;

        std::list<std::string>::iterator iterRVIPS = iter3->second.m_remoteVips.begin();
        for (; iterRVIPS != iter3->second.m_remoteVips.end(); ++iterRVIPS)
        {
            std::cout << "************************************* m_remoteVips: " << *iterRVIPS << std::endl;
        }
    }

    std::cout << "Changed list" << std::endl;
    std::map<std::string, SecurityAssociationItem>::iterator iter4 = saChangedList.m_mapItems.begin();
    for (; iter4 != saChangedList.m_mapItems.end(); ++iter4)
    {
        std::cout << "***************************** m_remoteId:   " << iter4->second.m_remoteId << std::endl;
        std::cout << "***************************** m_remoteHost: " << iter4->second.m_remoteHost << std::endl;
        std::cout << "***************************** m_localId:    " << iter4->second.m_localId << std::endl;
        std::cout << "***************************** m_localHost:  " << iter4->second.m_localHost << std::endl;

        std::list<std::string>::iterator iterRVIPS = iter4->second.m_remoteVips.begin();
        for (; iterRVIPS != iter4->second.m_remoteVips.end(); ++iterRVIPS)
        {
            std::cout << "************************************* m_remoteVips: " << *iterRVIPS << std::endl;
        }
    }

    std::cout << "Deleted list" << std::endl;
    std::map<std::string, SecurityAssociationItem>::iterator iter5 = saDeletedList.m_mapItems.begin();
    for (; iter5 != saDeletedList.m_mapItems.end(); ++iter5)
    {
        std::cout << "***************************** m_remoteId:   " << iter5->second.m_remoteId << std::endl;
        std::cout << "***************************** m_remoteHost: " << iter5->second.m_remoteHost << std::endl;
        std::cout << "***************************** m_localId:    " << iter5->second.m_localId << std::endl;
        std::cout << "***************************** m_localHost:  " << iter5->second.m_localHost << std::endl;

        std::list<std::string>::iterator iterRVIPS = iter5->second.m_remoteVips.begin();
        for (; iterRVIPS != iter5->second.m_remoteVips.end(); ++iterRVIPS)
        {
            std::cout << "************************************* m_remoteVips: " << *iterRVIPS << std::endl;
        }
    }

    m_saCompleteList.Add(saAddedList);
    m_saCompleteList.Remove(saDeletedList);
    m_saCompleteList.Remove(saChangedList);
    m_saCompleteList.Add(saChangedList);
    m_saNewList.Empty();

    std::cout << "m_saCompleteList: " << m_saCompleteList.Size() << std::endl;
}