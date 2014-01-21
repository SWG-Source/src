// ======================================================================
//
// StationPlayersCollector.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#include "StationPlayersCollector.h"
#include "StationPlayersCollectorImpl.h"
#include "ConfigStationPlayersCollector.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"

// ======================================================================

void StationPlayersCollector::install()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollector::install().\n"));

	ConfigStationPlayersCollector::install();
	
	ServerBase::install(new StationPlayersCollectorImpl);
	
	ServerBase::setSleepTimePerFrameMs(250);

	ExitChain::add(remove, "StationPlayersCollector::remove");
}

//-----------------------------------------------------------------------

void StationPlayersCollector::remove()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollector::remove().\n"));
	
	ServerBase::remove();

	ConfigStationPlayersCollector::remove();
}

//-----------------------------------------------------------------------

StationPlayersCollectorImpl *  StationPlayersCollector::getImpl()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollector::getImpl().\n"));

	return static_cast<StationPlayersCollectorImpl*>(s_implementation);
}

//=======================================================================
// game server message handlers
//=======================================================================
void StationPlayersCollector::handleSPCharacterProfileData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterProfileData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterItemData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterItemData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterVendorData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterVendorData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterVendorItemData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterVendorItemData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterStructureData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterStructureData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterShipData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterShipData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterSkillData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterSkillData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterMount_VehicleData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterMount_VehicleData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterWayPointData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterWayPointData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterQuestData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterQuestData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterSchematicData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterSchematicData(message);
}

//-----------------------------------------------------------------------

void StationPlayersCollector::handleSPCharacterRankingData(const GameNetworkMessage & message)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->handleSPCharacterRankingData(message);
}

//-----------------------------------------------------------------------

//======================================================================
// game server message & connection event callbacks & sends
//======================================================================
void StationPlayersCollector::ackSPCharacterProfileData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterProfileData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterItemData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterItemData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterVendorData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterVendorData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterVendorItemData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterVendorItemData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterStructureData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterStructureData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterShipData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterShipData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterSkillData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterSkillData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterMount_VehicleData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterMount_VehicleData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterWayPointData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterWayPointData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterQuestData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterQuestData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterSchematicData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterSchematicData();
}

//-----------------------------------------------------------------------

void StationPlayersCollector::ackSPCharacterRankingData()
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->ackSPCharacterRankingData();
}

//-----------------------------------------------------------------------

//======================================================================
// game server generic/global commincation methods
//======================================================================
void StationPlayersCollector::sendToCentralServer(const uint32 serverId, const GameNetworkMessage & message, const bool reliable)
{
	DEBUG_FATAL( !getImpl(), ("StationPlayersCollector implementation not set"));
	getImpl()->sendToCentralServer(serverId, message, reliable);
}

//-----------------------------------------------------------------------

// ======================================================================
