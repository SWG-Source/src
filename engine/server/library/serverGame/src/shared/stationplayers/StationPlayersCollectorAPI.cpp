// ======================================================================
//
// StationPlayersCollectorAPI.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "StationPlayersCollectorAPI.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "serverGame/GameServer.h"
#include "serverGame/ConfigServerGame.h"
#include "serverNetworkMessages/SPCharacterProfileMessage.h"
#include "serverGame/CreatureObject.h"

// ======================================================================

void StationPlayersCollectorAPI::install()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollectorAPI] : StationPlayersCollectorAPI::install().\n"));
		
	ExitChain::add(remove, "StationPlayersCollectorAPI::remove");
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::remove()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollectorAPI] : StationPlayersCollectorAPI::remove().\n"));
	
}

//-----------------------------------------------------------------------

//=======================================================================
// message senders
//=======================================================================
void StationPlayersCollectorAPI::sendSPCharacterProfileData(const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots)
{

	if (!ConfigServerGame::getStationPlayersEnabled())
		return;
		
	SPCharacterProfileMessage msg(clusterName, characterId, characterName, objectName, x, y, z, sceneId, cash_balance, bank_balance, objectTemplateName, stationId, containedBy, createTime, playedTime, numLots);
	
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : StationPlayersCollectorAPI::sendSPCharacterProfileData().\n"));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t Clustername: %s\n",msg.GetClusterName().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t CharacterID: %s\n",msg.GetCharacterId().getValueString().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t CharacterName: %s\n",msg.GetCharacterName().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t XCoord: %f\n",msg.GetX()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t YCoord: %f\n",msg.GetY()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t ZCoord: %f\n",msg.GetZ()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t SceneId: %s\n",msg.GetSceneId().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t Cash: %f\n",msg.GetCash_Balance()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t Bank: %f\n",msg.GetBank_Balance()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t Template: %s\n",msg.GetObjectTemplateName().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t StationId: %d\n",msg.GetStationId()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t ContainedBy: %s\n",msg.GetContainedBy().getValueString().c_str()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t CreateTime: %d\n",msg.GetCreateTime()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t PlayedTime: %d\n",msg.GetPlayedTime()));
	DEBUG_REPORT_LOG(ConfigServerGame::getStationPlayersShowAllDebugInfo(), ("[StationPlayersCollectorAPI] : \t Lots: %d\n",msg.GetNumLots()));

	
	sendToCentralServer(msg);
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterItemData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterVendorData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterVendorItemData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterStructureData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterShipData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterSkillData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterMount_VehicleData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterWayPointData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterQuestData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterSchematicData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::sendSPCharacterRankingData()
{
}

//-----------------------------------------------------------------------

//======================================================================
// game server message & connection event callbons & sends
//======================================================================
void StationPlayersCollectorAPI::onSPCharacterProfileData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterItemData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterVendorData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterVendorItemData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterStructureData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterShipData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterSkillData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterMount_VehicleData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterWayPointData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterQuestData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterSchematicData()
{
}

//-----------------------------------------------------------------------

void StationPlayersCollectorAPI::onSPCharacterRankingData()
{
}

//-----------------------------------------------------------------------

//======================================================================
// central server generic/global commincation methods
//======================================================================
void StationPlayersCollectorAPI::sendToCentralServer(const GameNetworkMessage & message)
{
	GameServer::getInstance().sendToCentralServer(message);
}

//-----------------------------------------------------------------------

// ======================================================================


