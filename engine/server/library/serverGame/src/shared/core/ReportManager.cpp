// ============================================================================
// 
// ReportManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ReportManager.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Os.h"
#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverUtility/ChatLogManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageParticipant.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/ChatRequestLog.h"
#include "sharedObject/Object.h"
#include "sharedObject/NetworkIdManager.h"
#include "serverUtility/ConfigServerUtility.h"
#include "UnicodeUtils.h"

// ============================================================================
//
// ReportManagerNamespace
//
// ============================================================================

namespace ReportManagerNamespace
{
	unsigned int s_sequenceId = 1;

	struct ReportData
	{
		Unicode::String m_reportingName;
		Unicode::String m_harassingName;
		NetworkId       m_reportingNetworkId;
		NetworkId       m_harassingNetworkId;
		std::string     m_reportLogChannel;
		Unicode::String m_clientData;
	};

	typedef std::map<unsigned int, ReportData> ReportList;
	ReportList s_reportList;

	typedef std::map<NetworkId, time_t> ReportThrottle;
	ReportThrottle s_reportThrottle;

	time_t const s_throttleTime = 10; // seconds
	time_t const s_throttleCleanupTime = 60; // seconds
	time_t s_throttleCleanupTimer = 0; // seconds
}

using namespace ReportManagerNamespace;

// ============================================================================
//
// ReportManager
//
// ============================================================================

//-----------------------------------------------------------------------------
bool ReportManager::isThrottled(NetworkId const &reportingNetworkId)
{
	bool result = false;
	ReportThrottle::iterator iterReportThrottle = s_reportThrottle.find(reportingNetworkId);

	if (iterReportThrottle != s_reportThrottle.end())
	{
		if ((Os::getRealSystemTime() - iterReportThrottle->second) >= s_throttleTime)
		{
			// Enough time elapsed

			s_reportThrottle.erase(iterReportThrottle);
		}
		else
		{
			// Still waiting to time out

			result = true;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
int ReportManager::getThrottleTimeRemaining(NetworkId const &reportingNetworkId)
{
	int result = 0;
	ReportThrottle::iterator iterReportThrottle = s_reportThrottle.find(reportingNetworkId);

	if (iterReportThrottle != s_reportThrottle.end())
	{
	    result = static_cast<int>(s_throttleTime - (Os::getRealSystemTime() - iterReportThrottle->second));
	}

	return result;
}

//-----------------------------------------------------------------------------
void ReportManager::addReport(Unicode::String const &reportingName, NetworkId const &reportingNetworkId, Unicode::String const &harassingName, NetworkId const &harassingNetworkId, std::string const &reportLogChannel, Unicode::String const & clientSentData)
{
	// Remove any expired throttling

	time_t const systemTime = Os::getRealSystemTime();

	if ((systemTime - s_throttleCleanupTimer) > s_throttleCleanupTime)
	{
		s_throttleCleanupTimer = systemTime;

		ReportThrottle::iterator iterReportThrottle = s_reportThrottle.begin();

		for (; iterReportThrottle != s_reportThrottle.end(); ++iterReportThrottle)
		{
			if (((systemTime - iterReportThrottle->second) >= s_throttleTime))
			{
				s_reportThrottle.erase(iterReportThrottle);
			}
		}
	}

	// Add this report to be throttled

	s_reportThrottle[reportingNetworkId] = systemTime;

	// Send the request to the chat server

	ChatAvatarId reportingChatAvatarId(Chat::constructChatAvatarId(Unicode::wideToNarrow(reportingName)));

	ChatRequestLog msg(s_sequenceId, Unicode::narrowToWide(reportingChatAvatarId.getFullName()));
	Chat::sendToChatServer(msg);

	ReportData reportData;
	reportData.m_reportingName = reportingName;
	reportData.m_harassingName = harassingName;
	reportData.m_reportingNetworkId = reportingNetworkId;
	reportData.m_harassingNetworkId = harassingNetworkId;
	reportData.m_clientData = clientSentData;

	if (reportLogChannel.empty())
		reportData.m_reportLogChannel = std::string("Report:");
	else
		reportData.m_reportLogChannel = reportLogChannel + std::string(":");

	s_reportList.insert(std::make_pair(s_sequenceId, reportData));

	++s_sequenceId;

	// Do we need to add a timeout period to let the player know if the chat server is down?
}

//-----------------------------------------------------------------------------
void ReportManager::handleMessage(ChatOnRequestLog const &chatOnRequestLog)
{
	// This function is the callback from receiving the report log request
	// to the chat server

	ReportList::const_iterator iterReportList = s_reportList.find(chatOnRequestLog.getSequence());

	if (iterReportList != s_reportList.end())
	{
		ReportData const &reportData = iterReportList->second;
		Unicode::String report;
		Unicode::String header;
		std::string const reportingPlayer(Unicode::toLower(Chat::constructChatAvatarId(Unicode::wideToNarrow(reportData.m_reportingName)).getFullName()));
		std::string const harassingPlayer(Unicode::toLower(Chat::constructChatAvatarId(Unicode::wideToNarrow(reportData.m_harassingName)).getFullName()));

		std::string reportingPlayerStationName;
		uint32 reportingPlayerStationId = 0;
		const CreatureObject * reporter = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(reportData.m_reportingNetworkId));
		if (reporter)
		{
			const Client * reporterClient = reporter->getClient();
			if (reporterClient)
			{
				reportingPlayerStationName = reporterClient->getAccountName();
				reportingPlayerStationId = reporterClient->getStationId();
			}
			else
			{
				const PlayerObject * reporterPlayer = PlayerCreatureController::getPlayerObject(reporter);
				if (reporterPlayer)
				{
					reportingPlayerStationId = reporterPlayer->getStationId();
				}
			}
		}

		std::string harassingPlayerStationName;
		uint32 harassingPlayerStationId = 0;
		const CreatureObject * harassing = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(reportData.m_harassingNetworkId));
		if (harassing)
		{
			const Client * harassingClient = harassing->getClient();
			if (harassingClient)
			{
				harassingPlayerStationName = harassingClient->getAccountName();
				harassingPlayerStationId = harassingClient->getStationId();
			}
			else
			{
				const PlayerObject * harassingPlayer = PlayerCreatureController::getPlayerObject(harassing);
				if (harassingPlayer)
				{
					harassingPlayerStationId = harassingPlayer->getStationId();
				}
			}
		}

		ChatLogManager::getReportHeader(header, reportingPlayer, reportData.m_reportingNetworkId, reportingPlayerStationName, reportingPlayerStationId, harassingPlayer, reportData.m_harassingNetworkId, harassingPlayerStationName, harassingPlayerStationId);

		report.append(header);

		typedef std::multimap<time_t, Unicode::String> SortedChatLog;
		typedef std::set<Unicode::String> SortedFromPlayers;
		SortedChatLog sortedChatLog;
		SortedFromPlayers sortedFromPlayers;

		// Sort the chat log

		std::vector<ChatLogEntry>::const_iterator iterLogEntries = chatOnRequestLog.getLogEntries().begin();

		{
			Unicode::String chatMessage;

			for (; iterLogEntries != chatOnRequestLog.getLogEntries().end(); ++iterLogEntries)
			{
				ChatLogEntry const &chatLogEntry = (*iterLogEntries);

				sortedFromPlayers.insert(chatLogEntry.m_from);
				ChatLogManager::buildLogEntry(chatMessage, chatLogEntry.m_from, chatLogEntry.m_to, chatLogEntry.m_message, chatLogEntry.m_channel, chatLogEntry.m_time);

				sortedChatLog.insert(std::make_pair(chatLogEntry.m_time, chatMessage));
			}
		}

		// Sort spatial chat

		CreatureObject * const reportingCreatureObject = CreatureObject::asCreatureObject(NetworkIdManager::getObjectById(reportData.m_reportingNetworkId));
		PlayerObject * const reportingPlayerObject = PlayerCreatureController::getPlayerObject(reportingCreatureObject);
		
		if (reportingPlayerObject != nullptr)
		{
			// Make sure the chat log does not have any extra data in it

			reportingPlayerObject->cleanChatLog();

			// Build the chat log string

			PlayerObject::ChatLog const &chatLog = reportingPlayerObject->getChatLog();
			PlayerObject::ChatLog::const_iterator iterChatLog = chatLog.begin();
			Unicode::String chatMessage;
			Unicode::String fromPlayer;
			Unicode::String toPlayer;
			Unicode::String text;
			Unicode::String channel;
			time_t time;

			for (; iterChatLog != chatLog.end(); ++iterChatLog)
			{
				int const chatMessageIndex = iterChatLog->m_index;

				if (ChatLogManager::getChatMessage(chatMessageIndex, fromPlayer, toPlayer, text, channel, time))
				{
					sortedFromPlayers.insert(fromPlayer);
					ChatLogManager::buildLogEntry(chatMessage, fromPlayer, toPlayer, text, channel, time);

					sortedChatLog.insert(std::make_pair(iterChatLog->m_time, chatMessage));
				}
				else
				{
					DEBUG_WARNING(true, ("Unable to find the chat message(%i)", chatMessageIndex));
				}
			}
		}
		else
		{
			report.append(Unicode::narrowToWide(FormattedString<1024>().sprintf("Unable to resolve networkId(%s) to a PlayerObject, this probably means the player was logged out while a /report was in the process of being made.", reportData.m_reportingNetworkId.getValueString().c_str())));
		}

		// Make sure the harassing player's name is in one of the chat entries as a from player

		bool foundHarassingPlayer = false;
		SortedFromPlayers::const_iterator iterSortedFromPlayers = sortedFromPlayers.find(Unicode::narrowToWide(harassingPlayer));

		if (iterSortedFromPlayers != sortedFromPlayers.end())
		{
			foundHarassingPlayer = true;
		}

		if (   !foundHarassingPlayer
		    && (reportingCreatureObject != nullptr))
		{
			// No harassing player match
			
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("system_msg", "report_invalid_name");
			
			ProsePackageParticipant target;
			target.str = reportData.m_harassingName;
			prosePackage.target = target;
			prosePackage.digitInteger = ConfigServerUtility::getChatLogMinutes();
			
			Chat::sendSystemMessage(*reportingCreatureObject, prosePackage);

			return;
		}

		// Put the final report together
		
		SortedChatLog::const_iterator iterSortedChatLog = sortedChatLog.begin();
		int count = 0;
		
		for (; iterSortedChatLog != sortedChatLog.end(); ++iterSortedChatLog)
		{
			report.append(Unicode::narrowToWide(FormattedString<32>().sprintf("[%3d] ", ++count)));
			report.append(iterSortedChatLog->second);
		}

		if(!reportData.m_clientData.empty())
		{
			report.append(Unicode::narrowToWide("Client sent data:\n"));
			report.append(reportData.m_clientData);
		}

		//if (!reportData.m_reportLogChannel.empty())
		//	LOGU("CustomerService", (reportData.m_reportLogChannel.c_str()), report);
		//else
		//	LOGU("CustomerService", ("Report:"), report);
	
		if(reportingCreatureObject->getScriptObject())
		{
			ScriptParams p;
			p.addParam(reportData.m_harassingNetworkId);
			p.addParam(report);
			IGNORE_RETURN(reportingCreatureObject->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_PLAYER_REPORTED_CHAT, p));
		}
		
		// System message for success
		
		ProsePackage prosePackage;
		prosePackage.stringId = StringId("system_msg", "report_success");
		
		ProsePackageParticipant target;
		target.str = reportData.m_harassingName;
		prosePackage.target = target;
		
		if (reportingCreatureObject != nullptr)
		{
			Chat::sendSystemMessage(*reportingCreatureObject, prosePackage);
		}
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("ReportManager: Received a report sequence number that is invalid: %d", chatOnRequestLog.getSequence()));
	}
}

// ============================================================================
