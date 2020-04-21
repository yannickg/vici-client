#pragma once

#include <string>
#include <map>
#include <list>
#include <string.h>
#include <arpa/inet.h>

class SecurityAssociationItem
{
public:
    SecurityAssociationItem() { }
    ~SecurityAssociationItem() { }

    bool AnythingHasChanged(const SecurityAssociationItem& rhs)
    {
        return (m_localHost != rhs.m_localHost
                || m_localId != rhs.m_localId
                || m_remoteHost != rhs.m_remoteHost
                || m_remoteId != rhs.m_remoteId
                || (GetIPv4Address() && rhs.GetIPv4Address() && strcmp(GetIPv4Address(), rhs.GetIPv4Address()) != 0));
    }

    const char* GetIPv4Address() const
    {
        const char* ipv4Address = NULL;
        unsigned char buf[sizeof(struct in6_addr)];
        std::list<std::string>::const_iterator iter = m_remoteVips.begin();
        for (; iter != m_remoteVips.end(); ++iter)
        {
            int s = inet_pton(AF_INET, iter->c_str(), buf);
            if (s > 0)
            {
                ipv4Address = iter->c_str();
                break;
            }
        }

        return ipv4Address;
    }

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
    void Add(const SecurityAssociationList& newList);
    void Copy(const SecurityAssociationList& newList);
    void Remove(const SecurityAssociationList& newList);

    void Empty();
    int Size() const;

    std::map<std::string, SecurityAssociationItem> m_mapItems;
};
