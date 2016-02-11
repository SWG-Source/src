// CustomerServiceServerConnection.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#include "serverGame/FirstServerGame.h"
#include "serverGame/CustomerServiceServerConnection.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverUtility/ChatLogManager.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ChatRequestLog.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"

//----------------------------------------------------------------------
CustomerServiceServerConnection::CustomerServiceServerConnection(const std::string & a, const unsigned short p)
: ServerConnection(a, p, NetworkSetupData())
{
	LOG("CSServerConnection", ("Connection created...listening on (%s:%d)", a.c_str(), static_cast<int>(p)));
}

//-----------------------------------------------------------------------
CustomerServiceServerConnection::~CustomerServiceServerConnection()
{
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onConnectionClosed()
{
	LOG("CustomerServiceServerConnection", ("onConnectionClosed()"));

	GameServer::getInstance().clearCustomerServiceServerConnection();
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onConnectionOpened()
{
	LOG("CustomerServiceServerConnection", ("onConnectionOpened()"));
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage gameNetworkMessage(ri);

	//REPORT_LOG(true, ("GameServer: CustomerServiceServerConnection::onReceive() message(%s)\n", gameNetworkMessage.getCmdName().c_str()));

	if (gameNetworkMessage.isType("ChatRequestLog"))
	{
		// See if this game server knows about this player

		ri = message.begin();
		ChatRequestLog chatRequestLog(ri);
		Object * const reportingObject = NetworkIdManager::getObjectById(NetworkId(Unicode::wideToNarrow(chatRequestLog.getPlayer())));

		if (   (reportingObject != nullptr)
		    && reportingObject->isAuthoritative())
		{
			PlayerObject const * const reportingPlayerObject = PlayerCreatureController::getPlayerObject(CreatureObject::asCreatureObject(reportingObject));
			
			if (reportingPlayerObject != nullptr)
			{
				PlayerObject::ChatLog const &reportingPlayerChatLog = reportingPlayerObject->getChatLog();
				PlayerObject::ChatLog::const_iterator iterReportingPlayerChatLog = reportingPlayerChatLog.begin();
				std::vector<ChatLogEntry> resultChatLog;

				for (; iterReportingPlayerChatLog != reportingPlayerChatLog.end(); ++iterReportingPlayerChatLog)
				{
					int const chatMessageIndex = iterReportingPlayerChatLog->m_index;
					ChatLogEntry chatLogEntry;
					
					chatLogEntry.m_time = iterReportingPlayerChatLog->m_time;

					if (!ChatLogManager::getChatMessage(chatMessageIndex, chatLogEntry.m_message))
					{
						DEBUG_WARNING(true, ("Unable to find chat log message(%i)\n", chatMessageIndex));
					}
					
					resultChatLog.push_back(chatLogEntry);
				}

				LOG("CustomerServiceServerConnection", ("onReceive(ChatRequestLog) Sending logs to the customer service server."));

				// Send the chat log to the customer service server
				
				ChatOnRequestLog chatOnRequestLog(chatRequestLog.getSequence(), resultChatLog);
				
				send(chatOnRequestLog, true);
			}
			else
			{
				LOG("CustomerServiceServerConnection", ("onReceive(ChatRequestLog) Unable to resolve networkId(%s) to a PlayerObject.", Unicode::wideToNarrow(chatRequestLog.getPlayer()).c_str()));
			}
		}
	}
}

//-----------------------------------------------------------------------
