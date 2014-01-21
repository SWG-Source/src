// ======================================================================
//
// StationPlayersCollectorImpl.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#include "StationPlayersCollectorImpl.h"
#include "CentralServerConnection.h"
#include "DatabaseConnection.h"
#include "ConfigStationPlayersCollector.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "serverNetworkMessages/SPCharacterProfileMessage.h"

//-----------------------------------------------------------------------

StationPlayersCollectorImpl::StationPlayersCollectorImpl() :
		ServerBaseImpl(),
		m_centralServerService(0)
{

}

//-----------------------------------------------------------------------

StationPlayersCollectorImpl::~StationPlayersCollectorImpl()
{

}

//=======================================================================
// virtual overrides from ServerBaseImpl
//=======================================================================

void StationPlayersCollectorImpl::preMainLoopInit()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::preMainLoopInit().\n"));

	ServerBaseImpl::preMainLoopInit();
	
	// add additional pre main loop initilizations here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::postMainLoopCleanup()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::postMainLoopCleanup().\n"));

	// add additional post main loop clean here

	ServerBaseImpl::postMainLoopCleanup();
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::setupConnections()
{
	NetworkSetupData setup;
	setup.port = ConfigStationPlayersCollector::getCentralServerServiceBindPort();
	setup.bindInterface = ConfigStationPlayersCollector::getCentralServerServiceBindInterface();
	setup.maxConnections=500;

	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::setupConnections() - Starting database connection pool.\n"));	
	DatabaseConnection::connect();
    
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::setupConnections() - Starting Central Server Listening Service on %s:%d.\n",ConfigStationPlayersCollector::getCentralServerServiceBindInterface(), static_cast<int>(ConfigStationPlayersCollector::getCentralServerServiceBindPort())));
	m_centralServerService = new Service(ConnectionAllocator<CentralServerConnection>(), setup);

}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::unsetupConnections()
{
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::unsetupConnections() - shutting down the Central Server Listening Service.\n"));

	m_centralServerService = 0;
	delete m_centralServerService;
	
	DEBUG_REPORT_LOG(true, ("[StationPlayersCollector] : StationPlayersCollectorImpl::unsetupConnections() - Disconnecting from the Database.\n"));
	DatabaseConnection::disconnect();
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::update (real time)
{
	// add StationPlayersCollectorImpl logic here
	
	DatabaseConnection::update();
	
	
	// end StationPlayersCollectorImpl 
	
	ServerBaseImpl::update(time);
}

//=======================================================================
// game server message handlers
//=======================================================================
void StationPlayersCollectorImpl::handleSPCharacterProfileData(const GameNetworkMessage & message)
{
	//TODO: do any additional data conversion/normalization here
	   
	Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
	SPCharacterProfileMessage msg(ri);
	
	
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : StationPlayersCollectorImpl::handleSPCharacterProfileData().\n"));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t Clustername: %s\n",msg.GetClusterName().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t CharacterID: %s\n",msg.GetCharacterId().getValueString().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t CharacterName: %s\n",msg.GetCharacterName().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t XCoord: %f\n",msg.GetX()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t YCoord: %f\n",msg.GetY()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t ZCoord: %f\n",msg.GetZ()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t SceneId: %s\n",msg.GetSceneId().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t Cash: %f\n",msg.GetCash_Balance()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t Bank: %f\n",msg.GetBank_Balance()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t Template: %s\n",msg.GetObjectTemplateName().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t StationId: %d\n",msg.GetStationId()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t ContainedBy: %s\n",msg.GetContainedBy().getValueString().c_str()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t CreateTime: %d\n",msg.GetCreateTime()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t PlayedTime: %d\n",msg.GetPlayedTime()));
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : \t Lots: %d\n",msg.GetNumLots()));
		
	DatabaseConnection::updateSPCharacterProfileData(
		msg.GetClusterName(),
		msg.GetCharacterId(),
		msg.GetCharacterName(),
		msg.GetObjectName(),
		msg.GetX(),
		msg.GetY(),
		msg.GetZ(),
		msg.GetSceneId(),
		msg.GetCash_Balance(),
		msg.GetBank_Balance(),
		msg.GetObjectTemplateName(),
		msg.GetStationId(),
		msg.GetContainedBy(),
		msg.GetCreateTime(),
		msg.GetPlayedTime(),
		msg.GetNumLots());
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterItemData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterVendorData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterVendorItemData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterStructureData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterShipData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterSkillData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterMount_VehicleData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterWayPointData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterQuestData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterSchematicData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::handleSPCharacterRankingData(const GameNetworkMessage & message)
{
	//TODO: add message handler code here
}

//-----------------------------------------------------------------------

//=======================================================================
// game server message & connection event callbacks & sends
//=======================================================================
void StationPlayersCollectorImpl::ackSPCharacterProfileData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterItemData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterVendorData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterVendorItemData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterStructureData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterShipData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterSkillData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterMount_VehicleData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterWayPointData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterQuestData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterSchematicData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

void StationPlayersCollectorImpl::ackSPCharacterRankingData()
{
	//TODO: add message callback code here
}

//-----------------------------------------------------------------------

//=======================================================================
// game server generic/global commincation methods
//=======================================================================

void StationPlayersCollectorImpl::sendToCentralServer(const uint32 serverId, const GameNetworkMessage & message, const bool reliable)
{
   DEBUG_WARNING(!reliable, ("[StationPlayersCollector] : Sending message unreliably from server to server"));

	CentralServerConnection * centralServerConnection =  CentralServerConnection::getConnectionById(serverId);
	if( centralServerConnection )
	{
		centralServerConnection->send( message, reliable );
	}
	else
	{
		DEBUG_REPORT_LOG(true,("[StationPlayersCollector] : Tried to send a message to a Central Server(%d), but it was not connected.\n", static_cast<int>(serverId)));
	}
}

// ======================================================================
