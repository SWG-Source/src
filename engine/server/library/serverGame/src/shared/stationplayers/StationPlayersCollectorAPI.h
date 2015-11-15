// ======================================================================
//
// StationPlayersCollectorAPI.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================
#ifndef	_INCLUDED_StationPlayersCollectorAPI_H
#define	_INCLUDED_StationPlayersCollectorAPI_H

//-----------------------------------------------------------------------

class NetworkId;
class GameNetworkMessage;

//-----------------------------------------------------------------------

class StationPlayersCollectorAPI 
{
 public:

	//===================================================================
	// send station players data sends 
	//===================================================================
	static void sendSPCharacterProfileData                     (const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots);
	static void sendSPCharacterItemData                        ();
	static void sendSPCharacterVendorData                      ();
	static void sendSPCharacterVendorItemData                  ();
	static void sendSPCharacterStructureData                   ();
	static void sendSPCharacterShipData                        ();
	static void sendSPCharacterSkillData                       ();
	static void sendSPCharacterMount_VehicleData               ();
	static void sendSPCharacterWayPointData                    ();
	static void sendSPCharacterQuestData                       ();
	static void sendSPCharacterSchematicData                   ();
	static void sendSPCharacterRankingData                     ();

	//===================================================================
	// API message & connection event callbacks & sends
	//===================================================================
	static void onSPCharacterProfileData                        ();
	static void onSPCharacterItemData                           ();
	static void onSPCharacterVendorData                         ();
	static void onSPCharacterVendorItemData                     ();
	static void onSPCharacterStructureData                      ();
	static void onSPCharacterShipData                           ();
	static void onSPCharacterSkillData                          ();
	static void onSPCharacterMount_VehicleData                  ();
	static void onSPCharacterWayPointData                       ();
	static void onSPCharacterQuestData                          ();
	static void onSPCharacterSchematicData                      ();
	static void onSPCharacterRankingData                        ();
	
	//===================================================================
	// game server generic/global commincation methods
	//===================================================================
	static void   sendToCentralServer(const GameNetworkMessage & message);

	static void install();
	static void remove();

 protected:

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_StationPlayersCollectorAPI_H
