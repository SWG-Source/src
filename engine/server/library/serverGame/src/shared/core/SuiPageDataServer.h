//======================================================================
//
// SuiPageDataServer.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiPageDataServer_H
#define INCLUDED_SuiPageDataServer_H

//======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/SuiPageData.h"

//----------------------------------------------------------------------

class SuiPageDataServer
{
public:

	explicit SuiPageDataServer(int pageId, std::string const & pageName, NetworkId const & ownerId);
	SuiPageDataServer & operator=(const SuiPageDataServer & rhs);
	SuiPageDataServer();
	SuiPageDataServer(const SuiPageDataServer & source);

	SuiPageData & getPageData();
	SuiPageData const & getPageData() const;
	NetworkId const & getOwnerId() const;
	bool isActive() const;

	void setActive();
	void markUpdateCommandIndex();
	bool getPageUpdate(SuiPageData & pageData);

public:

	bool operator==(SuiPageDataServer const & rhs) const;
	bool operator!=(SuiPageDataServer const & rhs) const;
	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;	

private:
	SuiPageData m_pageData;
	NetworkId m_ownerId;
	bool m_active;
	int m_updateCommandIndex;
};

//----------------------------------------------------------------------

inline SuiPageData & SuiPageDataServer::getPageData()
{
	return m_pageData;
}

//----------------------------------------------------------------------

inline SuiPageData const & SuiPageDataServer::getPageData() const
{
	return m_pageData;
}

//----------------------------------------------------------------------

inline NetworkId const & SuiPageDataServer::getOwnerId() const
{
	return m_ownerId;
}

//----------------------------------------------------------------------

inline bool SuiPageDataServer::operator!=(SuiPageDataServer const & rhs) const
{
	return !(*this==rhs);
}

//----------------------------------------------------------------------

inline bool SuiPageDataServer::isActive() const
{
	return m_active;
}

//======================================================================

#endif
