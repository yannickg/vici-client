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
