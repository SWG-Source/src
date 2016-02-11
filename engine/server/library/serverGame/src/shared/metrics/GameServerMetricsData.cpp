//GameServerMetricsData.cpp
//Copyright 2002 Sony Online Entertainment

#include "serverGame/FirstServerGame.h"
#include "serverGame/GameServerMetricsData.h"

#include "serverGame/GameServer.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ServerUIManager.h"
#include "serverGame/ServerUniverse.h"
#include "serverNetworkMessages/MetricsDataMessage.h"
#include "serverScript/GameScriptObject.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedObject/LotManager.h"
#include "serverGame/Client.h"
#include "sharedLog/Log.h"

//-----------------------------------------------------------------------



//-----------------------------------------------------------------------

GameServerMetricsData::GameServerMetricsData() :
MetricsData(),
m_numClients(0),
m_numBuildings(0),
m_numCreatures(0),
m_numSpawnLimit(0),
m_numInstallations(0),
m_numObjects(0),
m_numPlayers(0),
m_numRunTimeRules(0),
m_numTangibles(0),
m_numUniverseObjects(0),
m_numIntangibles(0),
m_numGroupObjects(0),
m_numWaypoints(0),
m_numAI(0),
m_numDynAI(0),
m_numStaticAI(0),
m_numCombatAI(0),
m_numHibernatingAI(0),
m_numDelayedHibernatingAI(0),
m_allocations(0),
m_gameProcessId(0),
m_noBuild(0),
m_numMessages(0),
m_numObjectsWithBackloggedMessages(0),
m_numObjectCreatesReceived(0),
m_numObjectCreatesSent(0),
m_freeJavaMemory(0),
m_numPendingLoadRequests(0),
m_numResourceTypesNative(0),
m_numResourceTypesImported(0)
{
	MetricsPair p;

	ADD_METRICS_DATA(numClients, 0, true);

	ADD_METRICS_DATA(numBuildings, 0, false);
	ADD_METRICS_DATA(numCreatures, 0, true);
	ADD_METRICS_DATA(numSpawnLimit, 0, false);
	ADD_METRICS_DATA(numInstallations, 0, false);
	ADD_METRICS_DATA(numObjects, 0, true);
	ADD_METRICS_DATA(numPlayers, 0, false);
	ADD_METRICS_DATA(numPlayersImperial, 0, true);
	ADD_METRICS_DATA(numPlayersRebel, 0, true);
	ADD_METRICS_DATA(numPlayersNeutral, 0, true);
	ADD_METRICS_DATA(numRunTimeRules, 0, false);
	ADD_METRICS_DATA(numTangibles, 0, false);
	ADD_METRICS_DATA(numUniverseObjects, 0, true);

	ADD_METRICS_DATA(numAI, 0, true);
	ADD_METRICS_DATA(numDynAI, 0, true);
	ADD_METRICS_DATA(numStaticAI, 0, true);
	ADD_METRICS_DATA(numCombatAI, 0, true);
	ADD_METRICS_DATA(numHibernatingAI, 0, true);
	ADD_METRICS_DATA(numDelayedHibernatingAI, 0, true);

	ADD_METRICS_DATA(numIntangibles, 0, false);
	ADD_METRICS_DATA(numGroupObjects, 0, false);
	ADD_METRICS_DATA(numWaypoints, 0, false);

	ADD_METRICS_DATA(gameProcessId, 0, false);

	ADD_METRICS_DATA(noBuild,0, false);

	ADD_METRICS_DATA(numMessages,0, false);
	ADD_METRICS_DATA(numObjectsWithBackloggedMessages,0, false);

	ADD_METRICS_DATA(numObjectCreatesReceived, 0, false);
	ADD_METRICS_DATA(numObjectCreatesSent, 0, false);

	ADD_METRICS_DATA(freeJavaMemory, 0, false);

	ADD_METRICS_DATA(numServerUIPages, 0, false);

	ADD_METRICS_DATA(numPendingLoadRequests, 0, false);

	ADD_METRICS_DATA(numResourceTypesNative, 0, false);
	ADD_METRICS_DATA(numResourceTypesImported, 0, false);

	std::string label = NetworkHandler::getHostName();

	for (std::string::iterator i = label.begin(); i != label.end(); ++i)
	{
		if (*i == '.')
		{
			*i = '+';
		}
	}

	label += ":";
	char tmp[256];
	label += _itoa(Os::getProcessId(), tmp, 10);

#ifndef WIN32
	FILE *inFile = ::fopen("/proc/cpuinfo", "rt");
	if (inFile)
	{
		char buffer[2048];
		while (::fgets(buffer, sizeof(buffer), inFile))
		{
			if (::strstr(buffer, "model name") == buffer)
			{
				buffer[::strcspn(buffer, "\r\n")] = '\0';

				char * c = buffer;
				while (*c)
				{
					if (::isspace(*c))
						*c = ' ';

					++c;
				}

				label += " (";
				label += buffer;
				label += ")";

				break;
			}
		}

		::fclose(inFile);
	}
#endif

	m_data[m_gameProcessId].m_description = label;
}

//-----------------------------------------------------------------------

GameServerMetricsData::~GameServerMetricsData()
{
}

//-----------------------------------------------------------------------

void GameServerMetricsData::updateData()
{
	MetricsData::updateData();
	m_data[m_numClients].m_value = GameServer::getInstance().getNumClients();

	m_data[m_numBuildings].m_value = ObjectTracker::getNumBuildings();
	m_data[m_numCreatures].m_value = ObjectTracker::getNumCreatures();

	// the following formula is also duplicated in utils.scriptlib::getNumCreaturesForSpawnLimit()
	m_data[m_numSpawnLimit].m_value = std::max(0, (ObjectTracker::getNumAI() - (ObjectTracker::getNumHibernatingAI() / 2)));

	static bool reportedServerSpawnLimit = false;
	if (!reportedServerSpawnLimit && (GameServer::getInstance().getPreloadAreaId() > 0))
	{
		char tmp[256];
		snprintf(tmp, sizeof(tmp)-1, "ServerSpawnLimit=%d", GameServer::getServerSpawnLimit());
		tmp[sizeof(tmp)-1] = '\0';
		m_data[m_numSpawnLimit].m_description = tmp;

		reportedServerSpawnLimit = true;
	}

	m_data[m_numInstallations].m_value = ObjectTracker::getNumInstallations();
	m_data[m_numObjects].m_value = ObjectTracker::getNumObjects();
	m_data[m_numPlayers].m_value = ObjectTracker::getNumPlayers();

	static int numPlayersImperial;
	static int numPlayersRebel;
	static int numPlayersNeutral;
	ObjectTracker::getNumPlayersByFaction(numPlayersImperial, numPlayersRebel, numPlayersNeutral);
	m_data[m_numPlayersImperial].m_value = numPlayersImperial;
	m_data[m_numPlayersRebel].m_value = numPlayersRebel;
	m_data[m_numPlayersNeutral].m_value = numPlayersNeutral;

	m_data[m_numRunTimeRules].m_value = ObjectTracker::getNumRunTimeRules();
	m_data[m_numTangibles].m_value = ObjectTracker::getNumTangibles();
	m_data[m_numUniverseObjects].m_value = ObjectTracker::getNumUniverseObjects();
	m_data[m_numIntangibles].m_value = ObjectTracker::getNumIntangibles();
	m_data[m_numGroupObjects].m_value = ObjectTracker::getNumGroupObjects();
	m_data[m_numWaypoints].m_value = ObjectTracker::getNumWaypoints();
	m_data[m_numAI].m_value = std::max(0, ObjectTracker::getNumAI());
	m_data[m_numDynAI].m_value = ObjectTracker::getNumDynamicAI();
	m_data[m_numStaticAI].m_value = ObjectTracker::getNumStaticAI();
	m_data[m_numCombatAI].m_value = ObjectTracker::getNumCombatAI();
	m_data[m_numHibernatingAI].m_value = ObjectTracker::getNumHibernatingAI();
	m_data[m_numDelayedHibernatingAI].m_value = ObjectTracker::getNumDelayedHibernatingAI();

	m_data[m_gameProcessId].m_value = static_cast<int>(GameServer::getInstance().getProcessId());

	const LotManager *lm = ServerWorld::getConstLotManager();
	if (lm)
		m_data[m_noBuild].m_value = lm->getNumberOfNoBuildEntries();
	else
		m_data[m_noBuild].m_value = 0;

	m_data[m_numMessages].m_value = MessageToQueue::getInstance().getTotalMessages();
	m_data[m_numObjectsWithBackloggedMessages].m_value = MessageToQueue::getInstance().getBackloggedObjectCount();

	m_data[m_numObjectCreatesReceived].m_value = static_cast<int>(GameServer::getInstance().getTotalObjectCreatesReceived());
	m_data[m_numObjectCreatesSent].m_value = static_cast<int>(GameServer::getInstance().getTotalObjectCreatesSent());

	m_data[m_freeJavaMemory].m_value = GameScriptObject::getScriptFreeMem();

	m_data[m_numServerUIPages].m_value = ServerUIManager::getNumServerUIPages();

	m_data[m_numPendingLoadRequests].m_value = GameServer::getNumberOfPendingLoadRequests();

	NetworkId id;
	unsigned long oldestPendingLoadRequestTime = GameServer::getOldestPendingLoadRequestTime(id);

	if (id.isValid())
		m_data[m_numPendingLoadRequests].m_description = FormattedString<256>().sprintf(
		"Max: %s requested load on %s which has been pending for %s. Limit = %d.",
		id.getValueString().c_str(),
		CalendarTime::convertEpochToTimeStringLocal(oldestPendingLoadRequestTime).c_str(),
		CalendarTime::convertSecondsToMS((int)::time(nullptr) - oldestPendingLoadRequestTime).c_str(),
		GameServer::getPendingLoadRequestLimit()
		);
	else
		m_data[m_numPendingLoadRequests].m_description = FormattedString<256>().sprintf(
		"No pending load requests. Limit = %d.",
		GameServer::getPendingLoadRequestLimit()
		);

	m_data[m_numResourceTypesNative].m_value = ServerUniverse::getInstance().getNumNativeResourceTypes();
	m_data[m_numResourceTypesImported].m_value = ServerUniverse::getInstance().getNumImportedResourceTypes();

/*****************  Disabled due to stats failing to update on live **********************
	std::map< std::string, uint32 >& cpmap = Client::getPacketBytesPerMinStats();
	std::map< std::string, uint32 >::iterator cpiter;

	// Packet metrics summary
	for ( cpiter = cpmap.begin(); cpiter != cpmap.end(); ++cpiter )
	{
		std::map< std::string, unsigned long>::iterator miter = m_packetDataMap.find( cpiter->first );
		if ( miter == m_packetDataMap.end() )  // If we haven't added this packet metric yet....
		{
			MetricsData* p_met = MetricsData::getInstance();
			std::string s_key("ClientPackets_BytesPerMin.");
			s_key+=cpiter->first;
			m_packetDataMap[ cpiter->first ] = p_met->addMetric(s_key.c_str(), 0, 0, true, true);   // add the ID to the local map
			miter = m_packetDataMap.find( cpiter->first );
		}
		m_data[ miter->second ].m_value = cpiter->second;
	}
***************************************************************************************/


}

//-----------------------------------------------------------------------
