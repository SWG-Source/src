// ======================================================================
//
// ClientMfdStatusUpdateMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ClientMfdStatusUpdateMessage.h"

#include <algorithm>

//-----------------------------------------------------------------------

const char * const ClientMfdStatusUpdateMessage::MESSAGE_TYPE = "ClientMfdStatusUpdateMessage";

//-----------------------------------------------------------------

ClientMfdStatusUpdateMessage::ClientMfdStatusUpdateMessage(const NetworkId & sourceId, const std::string & sceneName, const Vector & worldCoordinates, float updateTime, float updateDistance) :
GameNetworkMessage (MESSAGE_TYPE),
m_sceneName(sceneName),
m_sourceId(sourceId),
m_worldCoordinates(worldCoordinates),
m_hasChanged(true),
m_membersToUpdate(),
m_memberUpdateTimer(updateTime),
m_memberUpdateDistance(updateDistance * updateDistance)
{
	addVariable(m_sceneName);
	addVariable(m_sourceId);
	addVariable(m_worldCoordinates);
}

//-----------------------------------------------------------------------

ClientMfdStatusUpdateMessage::ClientMfdStatusUpdateMessage(Archive::ReadIterator & source) :
GameNetworkMessage (MESSAGE_TYPE),
m_sceneName(),
m_sourceId(),
m_worldCoordinates(),
m_hasChanged(true),
m_membersToUpdate(),
m_memberUpdateTimer(),
m_memberUpdateDistance()
{
	addVariable(m_sceneName);
	addVariable(m_sourceId);
	addVariable(m_worldCoordinates);
	unpack(source);
}

//----------------------------------------------------------------------

ClientMfdStatusUpdateMessage::~ClientMfdStatusUpdateMessage()
{
}

//-----------------------------------------------------------------------

std::vector<NetworkId> & ClientMfdStatusUpdateMessage::getMembersToUpdate()
{
	return m_membersToUpdate;
}

//-----------------------------------------------------------------------

bool ClientMfdStatusUpdateMessage::getHasChanged() const
{
	return m_hasChanged;
}

//-----------------------------------------------------------------------

const std::string & ClientMfdStatusUpdateMessage::getSceneName() const
{
	return m_sceneName.get();
}

//-----------------------------------------------------------------------

const NetworkId & ClientMfdStatusUpdateMessage::getSourceId() const
{
	return m_sourceId.get();
}

//-----------------------------------------------------------------------

const Vector & ClientMfdStatusUpdateMessage::getWorldCoordinates() const
{
	return m_worldCoordinates.get();
}

//-----------------------------------------------------------------------

void ClientMfdStatusUpdateMessage::addMemberToUpdate(const NetworkId & member)
{
	if(std::find(m_membersToUpdate.begin(), m_membersToUpdate.end(), member) == m_membersToUpdate.end())
	{
		m_membersToUpdate.push_back(member);
	}
}

//-----------------------------------------------------------------------

void ClientMfdStatusUpdateMessage::setSceneName(const std::string & sceneName)
{
	if(sceneName != m_sceneName.get())
	{
		m_hasChanged = true;
		m_sceneName.set(sceneName);
	}
}

//-----------------------------------------------------------------------

void ClientMfdStatusUpdateMessage::setWorldCoordinates(const Vector & worldCoordinates)
{
	if(worldCoordinates.magnitudeBetweenSquared(m_worldCoordinates.get()) > m_memberUpdateDistance)
	{
		m_hasChanged = true;
		m_worldCoordinates.set(worldCoordinates);
	}
}

//-----------------------------------------------------------------------

void ClientMfdStatusUpdateMessage::setHasChanged(bool hasChanged)
{
	m_hasChanged = hasChanged;
}

//-----------------------------------------------------------------------

bool ClientMfdStatusUpdateMessage::updateMemberUpdateTimer(float t)
{
	return m_memberUpdateTimer.updateZero(t);
}

//-----------------------------------------------------------------------
