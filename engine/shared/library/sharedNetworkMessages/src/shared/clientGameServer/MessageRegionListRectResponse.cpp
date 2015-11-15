// MessageRegionListRectResponse.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "MessageRegionListRectResponse.h"

//-----------------------------------------------------------------------

MessageRegionListRectResponse::MessageRegionListRectResponse(float worldX, float worldZ, float ur_worldX, float ur_worldZ, int pvp, int municipal, int buildable, int geographical, int minDifficulty, int maxDifficulty, int spawnable, int mission, const Unicode::String& name, const std::string& planet)
: GameNetworkMessage("MessageRegionListRectResponse"),
  m_name(name),
  m_planet(planet),
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
  m_ur_worldX(ur_worldX),
  m_ur_worldZ(ur_worldZ)
{
	addVariable(m_name);
	addVariable(m_planet);
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
	addVariable(m_radius);
	addVariable(m_ur_worldX);
	addVariable(m_ur_worldZ);
}

//-----------------------------------------------------------------------

MessageRegionListRectResponse::MessageRegionListRectResponse(Archive::ReadIterator & source)
: GameNetworkMessage("MessageRegionListRectResponse"),
  m_name(),
  m_planet(),
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
  m_ur_worldX(),
  m_ur_worldZ()
{
	addVariable(m_name);
	addVariable(m_planet);
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
	addVariable(m_radius);
	addVariable(m_ur_worldX);
	addVariable(m_ur_worldZ);
	unpack(source);
}

//-----------------------------------------------------------------------

MessageRegionListRectResponse::~MessageRegionListRectResponse()
{
}

//-----------------------------------------------------------------------

