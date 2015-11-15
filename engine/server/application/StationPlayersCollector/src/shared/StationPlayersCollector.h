// ======================================================================
//
// StationPlayersCollector.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================
#ifndef	_INCLUDED_StationPlayersCollector_H
#define	_INCLUDED_StationPlayersCollector_H

#include "FirstStationPlayersCollector.h"
#include "serverBase/ServerBase.h"

//-----------------------------------------------------------------------

class NetworkId;
class GameServerConnection;
class GameNetworkMessage;
class StationPlayersCollectorImpl;

//-----------------------------------------------------------------------

class StationPlayersCollector : public ServerBase
{
 public:

	//===================================================================
	// game server message handlers
	//===================================================================
	static void handleSPCharacterProfileData                     (const GameNetworkMessage & message);
	static void handleSPCharacterItemData                        (const GameNetworkMessage & message);
	static void handleSPCharacterVendorData                      (const GameNetworkMessage & message);
	static void handleSPCharacterVendorItemData                  (const GameNetworkMessage & message);
	static void handleSPCharacterStructureData                   (const GameNetworkMessage & message);
	static void handleSPCharacterShipData                        (const GameNetworkMessage & message);
	static void handleSPCharacterSkillData                       (const GameNetworkMessage & message);
	static void handleSPCharacterMount_VehicleData               (const GameNetworkMessage & message);
	static void handleSPCharacterWayPointData                    (const GameNetworkMessage & message);
	static void handleSPCharacterQuestData                       (const GameNetworkMessage & message);
	static void handleSPCharacterSchematicData                   (const GameNetworkMessage & message);
	static void handleSPCharacterRankingData                     (const GameNetworkMessage & message);

	//===================================================================
	// game server message & connection event callbacks & sends
	//===================================================================
	static void ackSPCharacterProfileData                        ();
	static void ackSPCharacterItemData                           ();
	static void ackSPCharacterVendorData                         ();
	static void ackSPCharacterVendorItemData                     ();
	static void ackSPCharacterStructureData                      ();
	static void ackSPCharacterShipData                           ();
	static void ackSPCharacterSkillData                          ();
	static void ackSPCharacterMount_VehicleData                  ();
	static void ackSPCharacterWayPointData                       ();
	static void ackSPCharacterQuestData                          ();
	static void ackSPCharacterSchematicData                      ();
	static void ackSPCharacterRankingData                        ();
	
	//===================================================================
	// game server generic/global commincation methods
	//===================================================================
	static void   sendToCentralServer(const uint32 serverId, const GameNetworkMessage & message, const bool reliable);

	static void install();
	static void remove();

 protected:

	static StationPlayersCollectorImpl * getImpl();

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_StationPlayersCollector_H
