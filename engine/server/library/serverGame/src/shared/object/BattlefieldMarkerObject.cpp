// ======================================================================
//
// BattlefieldMarkerObject.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BattlefieldMarkerObject.h"

#include "serverGame/BattlefieldMarkerController.h"
#include "serverGame/Pvp.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerBattlefieldMarkerObjectTemplate.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

static std::map<std::string, BattlefieldMarkerObject *> s_markerMap;

// ======================================================================

static void removeFromMarkerMap(std::string const &regionName, BattlefieldMarkerObject *marker)
{
	if (regionName.length())
	{
		std::map<std::string, BattlefieldMarkerObject*>::iterator i = s_markerMap.find(regionName);
		if (i != s_markerMap.end() && (*i).second == marker)
		{
			s_markerMap.erase(i);
			RegionMaster::registerBattlefieldMarker(regionName, 0);
		}
	}
}

// ----------------------------------------------------------------------

static bool addToMarkerMap(std::string const &regionName, BattlefieldMarkerObject *marker)
{
	if (regionName.length())
	{
		std::map<std::string, BattlefieldMarkerObject*>::iterator i = s_markerMap.find(regionName);
		if (i != s_markerMap.end())
			return false;
		s_markerMap[regionName] = marker;
		RegionMaster::registerBattlefieldMarker(regionName, marker);
	}
	return true;
}

// ======================================================================

BattlefieldMarkerObject::BattlefieldMarkerObject(ServerBattlefieldMarkerObjectTemplate const *newTemplate) :
	TangibleObject(newTemplate),
	m_regionName(),
	m_battlefieldParticipants()
{
	addMembersToPackages();

	m_regionName.setSourceObject(this);
	m_battlefieldParticipants.setOnSet(this, &BattlefieldMarkerObject::onParticipantSet);
	m_battlefieldParticipants.setOnInsert(this, &BattlefieldMarkerObject::onParticipantInsert);
	m_battlefieldParticipants.setOnErase(this, &BattlefieldMarkerObject::onParticipantErase);
}

// ----------------------------------------------------------------------

BattlefieldMarkerObject::~BattlefieldMarkerObject()
{
	removeFromMarkerMap(m_regionName.get(), this);
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::onLoadedFromDatabase()
{
	setPlacing(true);
	TangibleObject::onLoadedFromDatabase();
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::onRegionNameChanged(std::string const &oldName, std::string const &newName)
{
	removeFromMarkerMap(oldName, this);
	if (!addToMarkerMap(newName, this))
	{
		WARNING(true, ("Tried to create a duplicate battlefield marker %s, destroying it.", newName.c_str()));
		permanentlyDestroy(DeleteReasons::SetupFailed);
	}
}

// ----------------------------------------------------------------------

Controller *BattlefieldMarkerObject::createDefaultController()
{
	Controller * _controller = new BattlefieldMarkerController(this);

	setController(_controller);
	return _controller;
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::setRegionName(std::string const &newName)
{
	m_regionName.set(newName);
}

// ----------------------------------------------------------------------

std::string const &BattlefieldMarkerObject::getRegionName() const
{
	return m_regionName.get();
}

// ----------------------------------------------------------------------

std::map<NetworkId, uint32> const &BattlefieldMarkerObject::getBattlefieldParticipants() const
{
	return m_battlefieldParticipants.getMap();
}

// ----------------------------------------------------------------------

uint32 BattlefieldMarkerObject::getBattlefieldFactionId(NetworkId const &who) const
{
	std::map<NetworkId, uint32>::const_iterator i = m_battlefieldParticipants.find(who);
	if (i != m_battlefieldParticipants.end())
		return (*i).second;
	return 0;
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::setBattlefieldParticipant(NetworkId const &who, uint32 factionId)
{
	if (!isAuthoritative())
		sendControllerMessageToAuthServer(CM_setBattlefieldParticipant, new MessageQueueGenericValueType<std::pair<NetworkId, uint32> >(std::make_pair(who, factionId)));
	else if (factionId)
		m_battlefieldParticipants.set(who, factionId);
	else
		m_battlefieldParticipants.erase(who);
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::clearBattlefieldParticipants()
{
	if (!isAuthoritative())
		sendControllerMessageToAuthServer(CM_clearBattlefieldParticipants, 0);
	else
		m_battlefieldParticipants.clear();
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::onParticipantSet(NetworkId const &who, uint32 const &, uint32 const &)
{
	TangibleObject *tangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(who));
	if (tangible)
		Pvp::forceStatusUpdate(*tangible);
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::onParticipantInsert(NetworkId const &who, uint32 const &)
{
	TangibleObject *tangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(who));
	if (tangible)
		Pvp::forceStatusUpdate(*tangible);
}

// ----------------------------------------------------------------------

void BattlefieldMarkerObject::onParticipantErase(NetworkId const &who, uint32 const &)
{
	TangibleObject *tangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(who));
	if (tangible)
		Pvp::forceStatusUpdate(*tangible);
}

// ----------------------------------------------------------------------

BattlefieldMarkerObject *BattlefieldMarkerObject::findMarkerByRegionName(std::string const &regionName) // static
{
	std::map<std::string, BattlefieldMarkerObject *>::iterator i = s_markerMap.find(regionName);
	if (i != s_markerMap.end())
		return (*i).second;
	return 0;
}

// ======================================================================

