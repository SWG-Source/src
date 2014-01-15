//======================================================================
//
// SuiPageDataServer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SuiPageDataServer.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/SuiPageDataArchive.h"

//======================================================================

SuiPageDataServer::SuiPageDataServer(int pageId, std::string const & pageName, NetworkId const & ownerId) :
m_pageData(pageId, pageName),
m_ownerId(ownerId),
m_active(false),
m_updateCommandIndex(0)
{
}

//----------------------------------------------------------------------

SuiPageDataServer::SuiPageDataServer() :
m_pageData(),
m_ownerId(),
m_active(false),
m_updateCommandIndex(0)
{
}

//----------------------------------------------------------------------

SuiPageDataServer & SuiPageDataServer::operator =(const SuiPageDataServer & rhs)
{
	if(this != &rhs)
	{
		m_pageData = rhs.m_pageData;
		m_ownerId = rhs.m_ownerId;
		m_active = rhs.m_active;
		m_updateCommandIndex = rhs.m_updateCommandIndex;
	}
	return *this;
}

//----------------------------------------------------------------------

SuiPageDataServer::SuiPageDataServer(const SuiPageDataServer& in) :
m_pageData(in.m_pageData),
m_ownerId(in.m_ownerId),
m_active(in.m_active),
m_updateCommandIndex(in.m_updateCommandIndex)
{
}

//----------------------------------------------------------------------

void SuiPageDataServer::setActive()
{
	m_active = true;
}

//----------------------------------------------------------------------

void SuiPageDataServer::markUpdateCommandIndex()
{
	m_updateCommandIndex = static_cast<int>(m_pageData.getCommands().size());
}

//----------------------------------------------------------------------

bool SuiPageDataServer::getPageUpdate(SuiPageData & pageData)
{
	if (m_pageData.getPageUpdate(m_updateCommandIndex, pageData))
	{
		markUpdateCommandIndex();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void SuiPageDataServer::get(Archive::ReadIterator & source)
{
	Archive::get(source, m_pageData);
	Archive::get(source, m_ownerId);
	Archive::get(source, m_active);
	Archive::get(source, m_updateCommandIndex);
}

//----------------------------------------------------------------------

void SuiPageDataServer::put(Archive::ByteStream & target) const
{
	Archive::put(target, m_pageData);
	Archive::put(target, m_ownerId);
	Archive::put(target, static_cast<uint8>(m_active));
	Archive::put(target, m_updateCommandIndex);
}

//----------------------------------------------------------------------

bool SuiPageDataServer::operator==(SuiPageDataServer const & rhs) const
{
	return 
		m_pageData == rhs.m_pageData &&
		m_ownerId == rhs.m_ownerId &&
		m_active == rhs.m_active &&
		m_updateCommandIndex == rhs.m_updateCommandIndex;
}

//======================================================================
