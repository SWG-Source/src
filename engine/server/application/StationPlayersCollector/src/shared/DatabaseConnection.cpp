// ======================================================================
//
// DatabaseConnection.cpp
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#include "FirstStationPlayersCollector.h"
#include "DatabaseConnection.h"

#include "ConfigStationPlayersCollector.h"
#include "StationPlayersCollector.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "TaskUpdateSPCharacterProfileData.h"

// ======================================================================

namespace DatabaseConnectionNamespace
{
	DB::Server *s_databaseServer;
	DB::TaskQueue *s_taskQueue;
}

using namespace DatabaseConnectionNamespace;

// ======================================================================

void DatabaseConnection::connect()
{
	DEBUG_FATAL(s_databaseServer,("Called DatabaseConnection::connect() when already connected.\n"));
	
	s_databaseServer = DB::Server::create(ConfigStationPlayersCollector::getDSN(),
						ConfigStationPlayersCollector::getDatabaseUID(),
						ConfigStationPlayersCollector::getDatabasePWD(),
						DB::Server::getProtocolByName(ConfigStationPlayersCollector::getDatabaseProtocol()),true);
						
	DB::Server::setDisconnectSleepTime(ConfigStationPlayersCollector::getDisconnectSleepTime());
	s_taskQueue=new DB::TaskQueue(static_cast<unsigned int>(ConfigStationPlayersCollector::getDatabaseThreads()),s_databaseServer,0);

	if (ConfigStationPlayersCollector::getEnableQueryProfile())
		DB::Server::enableProfiling();

	if (ConfigStationPlayersCollector::getVerboseQueryMode())
		DB::Server::enableVerboseMode();

	DB::TaskQueue::enableWorkerThreadsLogging(ConfigStationPlayersCollector::getLogWorkerThreads());
}

// ----------------------------------------------------------------------

void DatabaseConnection::disconnect()
{
	DEBUG_FATAL(!s_databaseServer,("Called DatabaseConnection::disconnect() when not connected.\n"));

	if (ConfigStationPlayersCollector::getEnableQueryProfile())
	{
		DB::Server::debugOutputProfile();
		DB::Server::endProfiling();
	}
	
	delete s_taskQueue;
	delete s_databaseServer;

	s_taskQueue=0;
	s_databaseServer=0;
}

// ----------------------------------------------------------------------

void DatabaseConnection::update()
{
	NOT_NULL(s_taskQueue);
	s_taskQueue->update(ConfigStationPlayersCollector::getDefaultDBQueueUpdateTimeLimit());
}

// ----------------------------------------------------------------------

const std::string DatabaseConnection::getSchemaQualifier()
{
	if (ConfigStationPlayersCollector::getDatabaseSchema()[0]!='\0')
		return std::string(ConfigStationPlayersCollector::getDatabaseSchema())+'.';
	else
		return std::string();
}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterProfileData(const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots)
{
	NOT_NULL(s_taskQueue);
	s_taskQueue->asyncRequest(new TaskUpdateSPCharacterProfileData(clusterName,characterId,characterName,objectName,x,y,z,sceneId,cash_balance,bank_balance,objectTemplateName,stationId,containedBy,createTime,playedTime,numLots));
}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterItemData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterVendorData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterVendorItemData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterStructureData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterShipData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterSkillData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterMount_VehicleData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterWayPointData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterQuestData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterSchematicData()
{

}

// ----------------------------------------------------------------------

void DatabaseConnection::updateSPCharacterRankingData()
{

}

// ----------------------------------------------------------------------

// ======================================================================
