#include "SecurityAssociationList.h"


SecurityAssociationList::SecurityAssociationList()
{

}

SecurityAssociationList::~SecurityAssociationList()
{

}

void SecurityAssociationList::Add(const SecurityAssociationItem& saItem)
{
    m_mapItems.insert(std::pair<std::string, SecurityAssociationItem>(saItem.m_remoteId, saItem));
}

void SecurityAssociationList::Add(const SecurityAssociationList& newList)
{
    m_mapItems.insert(newList.m_mapItems.begin(), newList.m_mapItems.end());
}

void SecurityAssociationList::Empty()
{
    while (m_mapItems.size() > 0)
    {
        std::map<std::string, SecurityAssociationItem>::iterator iter = m_mapItems.begin();
        m_mapItems.erase(iter);
    }
}

void SecurityAssociationList::Remove(const SecurityAssociationList& newList)
{
    std::map<std::string, SecurityAssociationItem>::const_iterator iter1 = newList.m_mapItems.begin();
    for (; iter1 != newList.m_mapItems.end(); ++iter1)
    {
        std::map<std::string, SecurityAssociationItem>::iterator iter2 = m_mapItems.find(iter1->first);
        if (iter2 != m_mapItems.end())
        {
            m_mapItems.erase(iter2);
        }
    }
}

void SecurityAssociationList::Copy(const SecurityAssociationList& newList)
{
    m_mapItems = newList.m_mapItems;
}

int SecurityAssociationList::Size() const
 {
     return m_mapItems.size();
 }