// ======================================================================
//
// CreateDynamicRegionRectangleMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateDynamicRegionRectangleMessage.h"

// ======================================================================

CreateDynamicRegionRectangleMessage::CreateDynamicRegionRectangleMessage(float minX, float minZ, float maxX, float maxZ, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify) :
	GameNetworkMessage("CreateDynamicRegionRectangleMessage"),
	m_minX(minX),
	m_minZ(minZ),
	m_maxX(maxX),
	m_maxZ(maxZ),
	m_name(name),
	m_planet(planet),
	m_pvp(pvp),
	m_buildable(buildable),
	m_municipal(municipal),
	m_geography(geography),
	m_minDifficulty(minDifficulty),
	m_maxDifficulty(maxDifficulty),
	m_spawnable(spawnable),
	m_mission(mission),
	m_visible(visible),
	m_notify(notify)
{
	addVariable(m_minX);
	addVariable(m_minZ);
	addVariable(m_maxX);
	addVariable(m_maxZ);
	addVariable(m_name);
	addVariable(m_planet);
	addVariable(m_pvp);
	addVariable(m_buildable);
	addVariable(m_municipal);
	addVariable(m_geography);
	addVariable(m_minDifficulty);
	addVariable(m_maxDifficulty);
	addVariable(m_spawnable);
	addVariable(m_mission);
	addVariable(m_visible);
	addVariable(m_notify);
}

// ----------------------------------------------------------------------

CreateDynamicRegionRectangleMessage::CreateDynamicRegionRectangleMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateDynamicRegionRectangleMessage"),
	m_minX(),
	m_minZ(),
	m_maxX(),
	m_maxZ(),
	m_name(),
	m_planet(),
	m_pvp(),
	m_buildable(),
	m_municipal(),
	m_geography(),
	m_minDifficulty(),
	m_maxDifficulty(),
	m_spawnable(),
	m_mission(),
	m_visible(),
	m_notify()
{
	addVariable(m_minX);
	addVariable(m_minZ);
	addVariable(m_maxX);
	addVariable(m_maxZ);
	addVariable(m_name);
	addVariable(m_planet);
	addVariable(m_pvp);
	addVariable(m_buildable);
	addVariable(m_municipal);
	addVariable(m_geography);
	addVariable(m_minDifficulty);
	addVariable(m_maxDifficulty);
	addVariable(m_spawnable);
	addVariable(m_mission);
	addVariable(m_visible);
	addVariable(m_notify);
	unpack(source);
}

// ======================================================================

