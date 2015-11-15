// ======================================================================
//
// StationPlayersCollectorImpl.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================
#ifndef	_INCLUDED_StationPlayersCollectorImpl_H
#define	_INCLUDED_StationPlayersCollectorImpl_H

#include "FirstStationPlayersCollector.h"
#include "sharedFoundation/StationId.h"
#include "sharedFoundation/Tag.h"
#include "serverBase/ServerBaseImpl.h"

//-----------------------------------------------------------------------

class NetworkId;
class CentralServerConnection;
class Service;
class GameNetworkMessage;

//-----------------------------------------------------------------------

class StationPlayersCollectorImpl : public ServerBaseImpl
{
 public:

	virtual ~StationPlayersCollectorImpl();
	StationPlayersCollectorImpl();

	//===================================================================
	// game server loader message handlers
	//===================================================================
	virtual void handleSPCharacterProfileData                     (const GameNetworkMessage & message);
	virtual void handleSPCharacterItemData                        (const GameNetworkMessage & message);
	virtual void handleSPCharacterVendorData                      (const GameNetworkMessage & message);
	virtual void handleSPCharacterVendorItemData                  (const GameNetworkMessage & message);
	virtual void handleSPCharacterStructureData                   (const GameNetworkMessage & message);
	virtual void handleSPCharacterShipData                        (const GameNetworkMessage & message);
	virtual void handleSPCharacterSkillData                       (const GameNetworkMessage & message);
	virtual void handleSPCharacterMount_VehicleData               (const GameNetworkMessage & message);
	virtual void handleSPCharacterWayPointData                    (const GameNetworkMessage & message);
	virtual void handleSPCharacterQuestData                       (const GameNetworkMessage & message);
	virtual void handleSPCharacterSchematicData                   (const GameNetworkMessage & message);
	virtual void handleSPCharacterRankingData                     (const GameNetworkMessage & message);

	//===================================================================
	// game server message & connection event callbacks & sends
	//===================================================================
	virtual void ackSPCharacterProfileData                        ();
	virtual void ackSPCharacterItemData                           ();
	virtual void ackSPCharacterVendorData                         ();
	virtual void ackSPCharacterVendorItemData                     ();
	virtual void ackSPCharacterStructureData                      ();
	virtual void ackSPCharacterShipData                           ();
	virtual void ackSPCharacterSkillData                          ();
	virtual void ackSPCharacterMount_VehicleData                  ();
	virtual void ackSPCharacterWayPointData                       ();
	virtual void ackSPCharacterQuestData                          ();
	virtual void ackSPCharacterSchematicData                      ();
	virtual void ackSPCharacterRankingData                        ();
	
	//===================================================================
	// game server generic/global commincation methods
	//===================================================================
	virtual void   sendToCentralServer(const uint32 serverId, const GameNetworkMessage & message, const bool reliable);

	//===================================================================
	// The following are virtual overrides from BaseServerImpl
	//===================================================================
	virtual void update                                           (real time);
	virtual void setupConnections                                 ();
	virtual void unsetupConnections                               ();
	virtual void preMainLoopInit                                  ();
	virtual void postMainLoopCleanup                              ();
	
 protected:

	Service *                                    m_centralServerService;

};


//-----------------------------------------------------------------------

#endif	// _INCLUDED_StationPlayersCollector_H
