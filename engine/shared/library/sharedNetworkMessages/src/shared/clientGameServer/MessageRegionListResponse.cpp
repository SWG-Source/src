// MessageRegionListResponse.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "MessageRegionListResponse.h"

//-----------------------------------------------------------------------

MessageRegionListResponse::MessageRegionListResponse(const NetworkId& networkId, int gameServerId, float worldX, float worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String& label)
: GameNetworkMessage("MessageRegionListResponse"),
  m_objectId(networkId),
  m_label(label),
  m_worldX(worldX),
  m_worldZ(worldZ),
  m_pvp(pvp),
  m_buildable(buildable),
  m_spawnable(spawnable),
  m_mission(mission),
  m_municipal(municipal),
  m_geographical(geographical),
  m_minDifficulty(minDifficulty),
  m_maxDifficulty(maxDifficulty),
  m_gameServerId(gameServerId)
{
	addVariable(m_objectId);
	addVariable(m_label);
	addVariable(m_gameServerId);
	addVariable(m_worldX);
	addVariable(m_worldZ);
	addVariable(m_pvp);
	addVariable(m_buildable);
	addVariable(m_spawnable);
	addVariable(m_mission);
	addVariable(m_municipal);
	addVariable(m_geographical);
	addVariable(m_minDifficulty);
	addVariable(m_maxDifficulty);
}

//-----------------------------------------------------------------------

MessageRegionListResponse::MessageRegionListResponse(Archive::ReadIterator & source)
: GameNetworkMessage("MessageRegionListResponse"),
  m_objectId(),
  m_label(),
  m_worldX(),
  m_worldZ(),
  m_pvp(),
  m_buildable(),
  m_spawnable(),
  m_mission(),
  m_municipal(),
  m_geographical(),
  m_minDifficulty(),
  m_maxDifficulty(),
  m_gameServerId()
{
	addVariable(m_objectId);
	addVariable(m_label);
	addVariable(m_gameServerId);
	addVariable(m_worldX);
	addVariable(m_worldZ);
	addVariable(m_pvp);
	addVariable(m_buildable);
	addVariable(m_spawnable);
	addVariable(m_mission);
	addVariable(m_municipal);
	addVariable(m_geographical);
	addVariable(m_minDifficulty);
	addVariable(m_maxDifficulty);
	unpack(source);
}

//-----------------------------------------------------------------------

MessageRegionListResponse::MessageRegionListResponse(const std::string& messageName, const NetworkId& networkId, int gameServerId, float worldX, float worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String& label)
: GameNetworkMessage(messageName.c_str()),
  m_objectId(networkId),
  m_label(label),
  m_worldX(worldX),
  m_worldZ(worldZ),
  m_pvp(pvp),
  m_buildable(buildable),
  m_spawnable(spawnable),
  m_mission(mission),
  m_municipal(municipal),
  m_geographical(geographical),
  m_minDifficulty(minDifficulty),
  m_maxDifficulty(maxDifficulty),
  m_gameServerId(gameServerId)
{
	addVariable(m_objectId);
	addVariable(m_label);
	addVariable(m_gameServerId);
	addVariable(m_worldX);
	addVariable(m_worldZ);
	addVariable(m_pvp);
	addVariable(m_buildable);
	addVariable(m_spawnable);
	addVariable(m_mission);
	addVariable(m_municipal);
	addVariable(m_geographical);
	addVariable(m_minDifficulty);
	addVariable(m_maxDifficulty);
}

//-----------------------------------------------------------------------

MessageRegionListResponse::~MessageRegionListResponse()
{
}

//-----------------------------------------------------------------------

