#pragma once

#include <string>
#include <map>
#include <list>

class SecurityAssociationItem
{
public:
    SecurityAssociationItem() { }
    ~SecurityAssociationItem() { }

    std::string m_localHost;
    std::string m_localId;
    std::string m_remoteHost;
    std::string m_remoteId;
    std::list<std::string> m_remoteVips;
};

class SecurityAssociationList
{
public:
    SecurityAssociationList();
    ~SecurityAssociationList();

    void Add(const SecurityAssociationItem& saItem);

    std::map<std::string, SecurityAssociationItem> m_mapItems;
};
