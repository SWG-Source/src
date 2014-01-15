// ======================================================================
//
// CreateDynamicRegionCircleMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateDynamicRegionCircleMessage.h"

// ======================================================================

CreateDynamicRegionCircleMessage::CreateDynamicRegionCircleMessage(float centerX, float centerZ, float radius, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify) :
	GameNetworkMessage("CreateDynamicRegionCircleMessage"),
	m_centerX(centerX),
	m_centerZ(centerZ),
	m_radius(radius),
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
	addVariable(m_centerX);
	addVariable(m_centerZ);
	addVariable(m_radius);
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

CreateDynamicRegionCircleMessage::CreateDynamicRegionCircleMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateDynamicRegionCircleMessage"),
	m_centerX(),
	m_centerZ(),
	m_radius(),
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
	addVariable(m_centerX);
	addVariable(m_centerZ);
	addVariable(m_radius);
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

