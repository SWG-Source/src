// ======================================================================
//
// RegionPvp.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/RegionPvp.h"
#include "boost/smart_ptr.hpp"
#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"

// ======================================================================

// battlefield initialization message handler
const static std::string BATTLEFIELD_MESSAGE_HANDLER("msgInitializeRegion");
const static std::string BATTLEFIELD_MESSAGE_PLANET("region_planet");
const static std::string BATTLEFIELD_MESSAGE_NAME("region_name");

// ======================================================================

RegionPvp::RegionPvp(float centerX, float centerY, float radius) :
	RegionCircle(centerX, centerY, radius),
	m_battlefieldMarker(0)
{
}

// ----------------------------------------------------------------------

RegionPvp::RegionPvp(float centerX, float centerY, float radius, const CachedNetworkId & dynamicRegionId) :
	RegionCircle(centerX, centerY, radius, dynamicRegionId),
	m_battlefieldMarker(0)
{
}

// ----------------------------------------------------------------------

Pvp::FactionId RegionPvp::getBattlefieldFactionId(const NetworkId & object) const
{
	if (!m_battlefieldMarker)
	{
		WARNING(true, ("RegionPvp::getBattlefieldFactionId has no marker object for battlefield region %s", Unicode::wideToNarrow(getName()).c_str()));
		return 0;
	}
	return m_battlefieldMarker->getBattlefieldFactionId(object);
}	// RegionPvp::getBattlefieldFactionId

// ----------------------------------------------------------------------

void RegionPvp::setBattlefieldParticipant(const NetworkId & object, Pvp::FactionId factionId) const
{
	if (!m_battlefieldMarker)
	{
		WARNING(true, ("RegionPvp::setBattlefieldParticipant has no marker object for battlefield region %s", Unicode::wideToNarrow(getName()).c_str()));
		return;
	}
	m_battlefieldMarker->setBattlefieldParticipant(object, factionId);
}	// RegionPvp::setBattlefieldParticipant

// ----------------------------------------------------------------------

void RegionPvp::clearBattlefieldParticipants() const
{
	if (!m_battlefieldMarker)
	{
		WARNING(true, ("RegionPvp::clearBattlefieldParticipants has no marker object for battlefield region %s", Unicode::wideToNarrow(getName()).c_str()));
		return;
	}
	m_battlefieldMarker->clearBattlefieldParticipants();
}	// RegionPvp::clearBattlefieldParticipants

// ----------------------------------------------------------------------

void RegionPvp::setBattlefieldMarker(BattlefieldMarkerObject *marker)
{
	if (m_battlefieldMarker != marker)
	{
		m_battlefieldMarker = marker;
		if (marker)
			initializeBattlefield();
	}
}

// ----------------------------------------------------------------------

void RegionPvp::checkBattlefieldMarker()
{
	if (!m_battlefieldMarker)
	{
		m_battlefieldMarker = BattlefieldMarkerObject::findMarkerByRegionName(Unicode::wideToNarrow(getName()));
		if (m_battlefieldMarker)
			initializeBattlefield();
	}
}

// ----------------------------------------------------------------------

void RegionPvp::initializeBattlefield()
{
	NOT_NULL(m_battlefieldMarker);

	ScriptParams params;
	params.addParam(getPlanet().c_str(), BATTLEFIELD_MESSAGE_PLANET);
	params.addParam(getName(), BATTLEFIELD_MESSAGE_NAME);

	ScriptDictionaryPtr dictionary;
	m_battlefieldMarker->getScriptObject()->makeScriptDictionary(params, dictionary);
	dictionary->serialize();

	MessageToQueue::getInstance().sendMessageToJava(
		m_battlefieldMarker->getNetworkId(),
		BATTLEFIELD_MESSAGE_HANDLER,
		dictionary->getSerializedData(),
		1,
		true);
}

// ----------------------------------------------------------------------

RegionPvp *RegionPvp::asRegionPvp()
{
	return this;
}

// ----------------------------------------------------------------------

RegionPvp const *RegionPvp::asRegionPvp() const
{
	return this;
}

// ======================================================================

