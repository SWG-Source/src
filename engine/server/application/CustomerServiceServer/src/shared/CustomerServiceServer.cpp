// CustomerServiceServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "FirstCustomerServiceServer.h"
#include "CustomerServiceServer.h"
#include "CustomerServiceServerMetricsData.h"

#include "CentralServerConnection.h"
#include "ChatServerConnection.h"
#include "ConfigCustomerServiceServer.h"
#include "ConnectionServerConnection.h"
#include "GameServerConnection.h"
#include "UnicodeUtils.h"

#include "serverUtility/ChatLogManager.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/ChatRequestLog.h"
#include "sharedNetworkMessages/ConnectPlayerResponseMessage.h"
#include "sharedNetworkMessages/CreateTicketResponseMessage.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "sharedNetworkMessages/RequestCategoriesResponseMessage.h"

#include <map>
#include <set>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceServerNamespace
//
///////////////////////////////////////////////////////////////////////////////

namespace CustomerServiceServerNamespace
{
	const unsigned int s_chatLogRequestTimeout = 45;
	CustomerServiceServerMetricsData * s_customerServiceServerMetricsData = 0;
}

using namespace CustomerServiceServerNamespace;

CustomerServiceServer *CustomerServiceServer::m_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceServer::PendingTicket
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

CustomerServiceServer::PendingTicket::PendingTicket()
 : m_networkId()
 , m_time(0)
 , m_ticket()
 , m_hiddenDetails()
 , m_harassingPlayerName()
 , m_suid(0)
 , m_chatServerChatLog()
 , m_chatServerChatLogReceived(false)
 , m_gameServerChatLog()
 , m_gameServerChatLogReceived(false)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceServer
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

CustomerServiceServer::CustomerServiceServer() :
m_callback(new MessageDispatch::Callback),
m_centralServerConnection(nullptr),
m_connectionServerSet(new ConnectionServerSet),
m_done(false),
m_csInterface(ConfigCustomerServiceServer::getCustomerServiceServerAddress(), ConfigCustomerServiceServer::getCustomerServiceServerPort(), ConfigCustomerServiceServer::getRequestTimeoutSeconds()),
m_gameServerService(nullptr),
m_chatServerService(nullptr),
m_nextSequenceId(0),
m_pendingTicketList(new PendingTicketList)
{
	SetupSharedLog::install("SwgCustServServer");

	s_customerServiceServerMetricsData = new CustomerServiceServerMetricsData;
	MetricsManager::install(s_customerServiceServerMetricsData, false, "CustomerServiceServer" , "", 0);
	m_centralServerConnection = new CentralServerConnection(
		ConfigCustomerServiceServer::getCentralServerAddress(),
		ConfigCustomerServiceServer::getCentralServerPort());

	m_callback->connect(*this, &CustomerServiceServer::onEnumerateServers);

	char text[512];
	snprintf(text, sizeof(text), "CustomerServiceServer() - ServerName(%s) ServerPort(%i) GameCode(%s)", ConfigCustomerServiceServer::getCustomerServiceServerAddress(), ConfigCustomerServiceServer::getCustomerServiceServerPort(), ConfigCustomerServiceServer::getGameCode());
	REPORT_LOG(true, ("%s\n", text));
	LOG("CSServer", ("%s", text));

	LOG("CSServer", ("CustomerServiceServer() - MaxPacketsPerSecond(%d)", ConfigCustomerServiceServer::getMaxPacketsPerSecond()));
	LOG("CSServer", ("CustomerServiceServer() - RequestTimeoutSeconds(%d)", ConfigCustomerServiceServer::getRequestTimeoutSeconds()));
	LOG("CSServer", ("CustomerServiceServer() - MaxAllowedNumberOfTickets(%d)", ConfigCustomerServiceServer::getMaxAllowedNumberOfTickets()));

	m_csInterface.setMaxPacketsPerSecond(ConfigCustomerServiceServer::getMaxPacketsPerSecond());

	m_csInterface.connectCSAssist(nullptr, 
		Unicode::narrowToWide(ConfigCustomerServiceServer::getGameCode()).data(), 
		Unicode::narrowToWide(ConfigCustomerServiceServer::getClusterName()).data());

	// Create the GameServer connection

	NetworkSetupData setup;
	setup.port = ConfigCustomerServiceServer::getGameServicePort();
	setup.maxConnections = 100;
	setup.bindInterface = ConfigCustomerServiceServer::getGameServiceBindInterface();
	m_gameServerService = new Service(ConnectionAllocator<GameServerConnection>(), setup);

	// Create the ChatServer connection

	setup.port = ConfigCustomerServiceServer::getChatServicePort();
	setup.bindInterface = ConfigCustomerServiceServer::getChatServiceBindInterface();
	m_chatServerService = new Service(ConnectionAllocator<ChatServerConnection>(), setup);
}

//-----------------------------------------------------------------------

CustomerServiceServer::~CustomerServiceServer()
{
	ConnectionServerSet::const_iterator i = m_connectionServerSet->begin();
	while (i != m_connectionServerSet->end())
	{
		ConnectionServerConnection *c = (*i);
		delete c;
		++i;
	}

	if(m_centralServerConnection)
		m_centralServerConnection->disconnect();

	delete m_callback;
	m_callback = nullptr;

	delete m_connectionServerSet;
	m_connectionServerSet = nullptr;

	delete m_gameServerService;
	m_gameServerService = nullptr;

	delete m_chatServerService;
	m_chatServerService = nullptr;

	delete m_pendingTicketList;
	m_pendingTicketList = nullptr;

	MetricsManager::remove();
	delete s_customerServiceServerMetricsData;

}

//-----------------------------------------------------------------------

void CustomerServiceServer::onEnumerateServers(const EnumerateServers &e)
{
	if (e.getAdd())
	{
		switch(e.getServerType())
		{
			case EnumerateServers::CONNECTION_SERVER:
			{
				ConnectionServerConnection *c = new ConnectionServerConnection(
					e.getAddress(), e.getPort());
				IGNORE_RETURN(m_connectionServerSet->insert(c));
				break;
			}

			default:
				break;
		}
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::reconnectToCentralServer()
{
	if (m_centralServerConnection)
	{
		m_centralServerConnection->disconnect();
	}
	m_centralServerConnection = new CentralServerConnection(
		ConfigCustomerServiceServer::getCentralServerAddress(),
		ConfigCustomerServiceServer::getCentralServerPort());
}

//-----------------------------------------------------------------------

void CustomerServiceServer::run() 
{
	Clock::setFrameRateLimit(100.0f);
	while(!isDone())
	{
		update();
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::update()
{
	m_csInterface.Update();
	NetworkHandler::update();
	NetworkHandler::dispatch();

	MetricsManager::update(Clock::frameTime() * 1000);
	processPendingTickets();

	if (!Os::update())
		m_done = true;
	Os::sleep(1);
}

//-----------------------------------------------------------------------

CustomerServiceServer &CustomerServiceServer::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new CustomerServiceServer;
	}
	return *m_instance;
}

//-----------------------------------------------------------------------

void CustomerServiceServer::requestCategories(NetworkId const & requester, std::string const & language)
{
	LOG("CSServer", ("requestCategories() networkId(%s) langauge(%s)", requester.getValueString().c_str(), language.c_str()));

	if (language == "ja")
	{
		RequestCategoriesResponseMessage const message(CSASSIST_RESULT_SUCCESS, m_csInterface.getJapaneseCategoryList());
		m_csInterface.sendToClient(requester, message);	
	}
	else
	{
		if (language != "en")
		{
			LOG("CSServer", ("ERROR: Unknown language(%s) specified...sending English categories to the client.", language.c_str()));
		}

		RequestCategoriesResponseMessage const message(CSASSIST_RESULT_SUCCESS, m_csInterface.getEnglishCategoryList());
		m_csInterface.sendToClient(requester, message);	
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::createTicket(
	const NetworkId &requester, 
	unsigned int suid,
	const std::string &characterName,
	unsigned int category,
	unsigned int subCategory,
	const Unicode::String &details,
	const Unicode::String &hiddenDetails,
	const Unicode::String &harassingCharacterName,
	const std::string &language,
	bool isBug
)
{
	int const ticketCount = m_csInterface.getTicketCount(requester);
	bool ticketAllowed = false;

	if (isBug)
	{
		ticketAllowed = true;

		if (ConfigCustomerServiceServer::getWriteTicketToBugLog())
		{
			FormattedString<8192> fs;

			char const * const logMessage = fs.sprintf(
				"Bugs:"
				"char_id=%s, "
				"character_name=%s, "
				"category=%d, "
				"subcategory=%d, "
				"language=%s\n"
				"%s",
				requester.getValueString().c_str(),
				characterName.c_str(),
				category,
				subCategory,
				language.c_str(),
				Unicode::wideToNarrow(details).c_str()
				);

			LOG("CustomerService", ("%s", logMessage));
		}
	}
	else if (ticketCount < 0)
	{
		LOG("CSServer", ("createTicket() - Ticket Creation Blocked for networkId(%s) suid(%i) because we have not received how many tickets this player has already.", requester.getValueString().c_str(), suid));

		CreateTicketResponseMessage const message(CSASSIST_RESULT_NOCREATE_TICKET, 0);
		m_csInterface.sendToClient(requester, message);	
	}
	else if (ticketCount < ConfigCustomerServiceServer::getMaxAllowedNumberOfTickets())
	{
		ticketAllowed = true;
	}
	else
	{
		LOG("CSServer", ("createTicket() - Ticket Creation Blocked for networkId(%s) suid(%i) ticket count(%d) max tickets allowed(%d)", requester.getValueString().c_str(), suid, ticketCount, ConfigCustomerServiceServer::getMaxAllowedNumberOfTickets()));

		CreateTicketResponseMessage const message(CSASSIST_RESULT_NOCREATE_TICKET, 0);
		m_csInterface.sendToClient(requester, message);	
	}

	if (ticketAllowed)
	{
		LOG("CSServer", ("createTicket() - networkId(%s) suid(%i) category(%i) subCategory(%i) isBug(%s)", requester.getValueString().c_str(), suid, category, subCategory, isBug ? "yes" : "no"));

		CSAssistGameAPITicket ticket;

		ticket.setUID(suid); 
		ticket.setGame(Unicode::narrowToWide(ConfigCustomerServiceServer::getGameCode()).data());
		ticket.setServer(Unicode::narrowToWide(ConfigCustomerServiceServer::getClusterName()).data());
		ticket.setCharacter(Unicode::narrowToWide(characterName).data());
		ticket.setDetails(details.c_str());
		ticket.setLanguage(Unicode::narrowToWide(language).data());
		ticket.setCategory(0, category);
		ticket.setCategory(1, subCategory); 

		if (isBug)
		{
			ticket.setBug();
		}

		bool const logChat = !harassingCharacterName.empty();

		createTicket(requester, ticket, hiddenDetails, harassingCharacterName, suid, logChat);
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::appendComment(
	const NetworkId &requester, 
	unsigned int suid, 
	const std::string &characterName, 
	unsigned int ticketId, 
	const Unicode::String &comment
)
{
	LOG("CSServer", ("appendComment() - networkId(%s) suid(%i) characterName(%s) ticketId(%i)", requester.getValueString().c_str(), suid, characterName.c_str(), ticketId));

	NetworkId *tmpNetworkId = new NetworkId(requester);
	m_csInterface.requestAppendTicketComment(
		reinterpret_cast<const void *>(tmpNetworkId), ticketId, suid,
		Unicode::narrowToWide(characterName.c_str()).data(), 
		comment.data());
}

//-----------------------------------------------------------------------

void CustomerServiceServer::cancelTicket(
	const NetworkId &requester, 
	unsigned int suid, 
	unsigned int ticketId, 
	const Unicode::String &comment
)
{
	NetworkId *tmpNetworkId = new NetworkId(requester);

	LOG("CSServer", ("cancelTicket() - networkId(%s) suid(%i) ticketId(%i)", requester.getValueString().c_str(), suid, ticketId));

	m_csInterface.requestCancelTicket(
		reinterpret_cast<const void *>(tmpNetworkId), ticketId, suid,
		comment.data());
}

//-----------------------------------------------------------------------

void CustomerServiceServer::getTickets(
	const NetworkId &requester, 
	unsigned int suid
)
{
	LOG("CSServer", ("getTickets() - networkId(%s) suid(%i)", requester.getValueString().c_str(), suid));

	bool const markAsRead = false;
	unsigned int const start = 0;
	unsigned int const count = 2000;

	NetworkId *tmpNetworkId = new NetworkId(requester);
	m_csInterface.requestGetTicketByCharacter(
		reinterpret_cast<const void *>(tmpNetworkId), suid, nullptr,
		start, count, markAsRead);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::getComments(
	const NetworkId &requester, 
	unsigned int ticketId 
)
{
	LOG("CSServer", ("getComments() - networkId(%s) ticketId(%i)", requester.getValueString().c_str(), ticketId));

	NetworkId *tmpNetworkId1 = new NetworkId(requester);
	m_csInterface.requestGetTicketComments(
		reinterpret_cast<const void *>(tmpNetworkId1), ticketId, 0,
		100, CSASSIST_OFFSET_START);

	NetworkId *tmpNetworkId2 = new NetworkId(requester);
	m_csInterface.requestMarkTicketRead(reinterpret_cast<const void *>(tmpNetworkId2), ticketId);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::searchKnowledgeBase(
	const NetworkId &requester, 
	const Unicode::String &searchString, 
	const std::string &language
)
{
	LOG("CSServer", ("searchKnowledgeBase() - networkId(%s)", requester.getValueString().c_str()));

	NetworkId *tmpNetworkId = new NetworkId(requester);
//	printf("Searching Knowledge base with %s, %s\n", 
//		Unicode::wideToNarrow(searchString).c_str(),
//		language.c_str());

	m_csInterface.requestSearchKB(
		reinterpret_cast<const void *>(tmpNetworkId), 
		searchString.data(),
		Unicode::narrowToWide(language).data(),
		0);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::getArticle(
	const NetworkId &requester, 
	const std::string &id, 
	const std::string &language
)
{
	LOG("CSServer", ("getArticle() - networkId(%s) id(%s) language(%s)", requester.getValueString().c_str(), id.c_str(), language.c_str()));

	NetworkId *tmpNetworkId = new NetworkId(requester);
	m_csInterface.requestGetKBArticle(
		reinterpret_cast<const void *>(tmpNetworkId), 
		Unicode::narrowToWide(id).data(), 
		Unicode::narrowToWide(language).data(),
		0);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::requestNewTicketActivity(const NetworkId &requester, unsigned int suid)
{
	LOG("CSServer", ("requestNewTicketActivity() - networkId(%s) suid(%i)", requester.getValueString().c_str(), suid));

	NetworkId *tmpNetworkId = new NetworkId(requester);

	m_csInterface.requestNewTicketActivity(reinterpret_cast<const void *>(tmpNetworkId), suid, nullptr);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::requestRegisterCharacter(const NetworkId &requester, ConnectionServerConnection *connection, const unsigned int suid)
{
	LOG("CSServer", ("requestRegisterCharacter() - networkId(%s) suid(%i)", requester.getValueString().c_str(), suid));

	m_csInterface.addPlayer(requester, connection, suid);

	// If the suid is not valid, don't send the request to the backend

	if (suid == 0)
	{
		LOG("CSServer", ("CustomerServiceInterface::requestRegisterCharacter() - FAILING due to invalid suid(%i) networkId(%s)", suid, requester.getValueString().c_str()));
	}
	else
	{
		LOG("CSServer", ("CustomerServiceInterface::requestRegisterCharacter() - networkId(%s) suid(%i)", requester.getValueString().c_str(), suid));

		NetworkId *tmpNetworkId = new NetworkId(requester);
		m_csInterface.requestRegisterCharacter(reinterpret_cast<const void *>(tmpNetworkId), suid, nullptr, 0);
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::requestUnRegisterCharacter(const NetworkId &requester)
{
	m_csInterface.requestUnRegisterCharacter(requester);
}

//-----------------------------------------------------------------------

void CustomerServiceServer::connectionServerConnectionDestroyed(ConnectionServerConnection *connectionServerConnection)
{
	m_csInterface.connectionServerConnectionDestroyed(connectionServerConnection);
}

//-----------------------------------------------------------------------

Service *CustomerServiceServer::getGameServerService() const
{
	return m_gameServerService;
}

//-----------------------------------------------------------------------

Service *CustomerServiceServer::getChatServerService() const
{
	return m_chatServerService;
}

//-----------------------------------------------------------------------

void CustomerServiceServer::createTicket(NetworkId const &networkId, const CSAssistGameAPITicket &ticket, Unicode::String const &hiddenDetails, Unicode::String const &harassingPlayerName, const unsigned int suid, const bool logChat)
{
	m_csInterface.incrementPendingTicketCount(networkId);

	if (logChat)
	{
		// Request the chat logs

		Unicode::String player;
		player.append(ticket.game);
		player.append(Unicode::narrowToWide("."));
		player.append(ticket.server);
		player.append(Unicode::narrowToWide("."));
		player.append(ticket.character);

		{
			ChatRequestLog const chatRequestLogMessage(m_nextSequenceId, player);
			ChatServerConnection::sendTo(chatRequestLogMessage);
		}
		{
			ChatRequestLog const chatRequestLogMessage(m_nextSequenceId, Unicode::narrowToWide(networkId.getValueString()));
			GameServerConnection::broadcast(chatRequestLogMessage);
		}

		// Store the ticket information locally

		PendingTicket pendingTicket;

		pendingTicket.m_networkId = networkId;
		pendingTicket.m_time = Os::getRealSystemTime();
		pendingTicket.m_ticket = ticket;
		pendingTicket.m_hiddenDetails = hiddenDetails;
		pendingTicket.m_harassingPlayerName = harassingPlayerName;
		pendingTicket.m_suid = suid;

		m_pendingTicketList->insert(std::make_pair(m_nextSequenceId, pendingTicket));

		++m_nextSequenceId;
	}
	else
	{
		// Go ahead and create the ticket

		LOG("CSServer", ("createTicket() name(%s) networkId(%s)", Unicode::wideToNarrow(ticket.character).c_str(), networkId.getValueString().c_str()));

		NetworkId *tmpNetworkId = new NetworkId(networkId);
		m_csInterface.requestCreateTicket(reinterpret_cast<const void *>(tmpNetworkId), &ticket, hiddenDetails.data(), suid);
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::handleChatServerChatLog(ChatOnRequestLog const &chatOnRequestLog)
{
	LOG("CSServer", ("handleChatServerChatLog() sequenceId(%d) entries(%d)", chatOnRequestLog.getSequence(), static_cast<int>(chatOnRequestLog.getLogEntries().size())));

	PendingTicketList::iterator iterPendingTicketList = m_pendingTicketList->find(chatOnRequestLog.getSequence());

	if (iterPendingTicketList != m_pendingTicketList->end())
	{
		LOG("CSServer", ("handleChatServerChatLog() ticket found!"));
		PendingTicket &pendingTicket = iterPendingTicketList->second;

		pendingTicket.m_chatServerChatLog = chatOnRequestLog.getLogEntries();
		pendingTicket.m_chatServerChatLogReceived = true;
	}
	else
	{
		LOG("CSServer", ("handleChatServerChatLog() ERROR: Unable to find owner for ChatServer chat log."));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::handleGameServerChatLog(ChatOnRequestLog const &chatOnRequestLog)
{
	LOG("CSServer", ("handleGameServerChatLog() sequenceId(%d) entries(%d)", chatOnRequestLog.getSequence(), static_cast<int>(chatOnRequestLog.getLogEntries().size())));
	PendingTicketList::iterator iterPendingTicketList = m_pendingTicketList->find(chatOnRequestLog.getSequence());

	if (iterPendingTicketList != m_pendingTicketList->end())
	{
		LOG("CSServer", ("handleGameServerChatLog() ticket found!"));
		PendingTicket &pendingTicket = iterPendingTicketList->second;

		pendingTicket.m_gameServerChatLog = chatOnRequestLog.getLogEntries();
		pendingTicket.m_gameServerChatLogReceived = true;
	}
	else
	{
		LOG("CSServer", ("handleGameServerChatLog() ERROR: Unable to find owner for GameServer chat log."));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::processPendingTickets()
{
	// See if any pending tickets are ready to be sent

	PendingTicketList::iterator iterPendingTicketList = m_pendingTicketList->begin();

	for (; iterPendingTicketList != m_pendingTicketList->end(); ++iterPendingTicketList)
	{
		PendingTicket &pendingTicket = iterPendingTicketList->second;

		SortedChatLog sortedChatLog;

		if ((Os::getRealSystemTime() - pendingTicket.m_time) > s_chatLogRequestTimeout)
		{
			// If it has taken over the specified seconds to get the ticket
			// chat logs, just send the ticket and mention that we were
			// unable to receive the chat logs

			if (!pendingTicket.m_chatServerChatLog.empty())
			{
				addChatLogs(sortedChatLog, pendingTicket.m_chatServerChatLog);
				LOG("CSServer", ("processPendingTickets() ChatServer chat log received"));
			}

			if (!pendingTicket.m_gameServerChatLog.empty())
			{
				addChatLogs(sortedChatLog, pendingTicket.m_gameServerChatLog);
				LOG("CSServer", ("processPendingTickets() GameServer chat log received"));
			}

			Unicode::String report;
			createReport(pendingTicket, sortedChatLog, report);

			Unicode::String hiddenDetails(pendingTicket.m_hiddenDetails);
			hiddenDetails.append(Unicode::narrowToWide("\n"));
			hiddenDetails.append(report);

			LOG("CSServer", ("processPendingTickets() TIMEOUT - Partial chat logs received"));
			m_csInterface.decrementPendingTicketCount(pendingTicket.m_networkId);
			
			bool const logChat = false;
			createTicket(pendingTicket.m_networkId, pendingTicket.m_ticket, hiddenDetails, Unicode::emptyString, pendingTicket.m_suid, logChat);

			// Bail, we will catch another ticket next frame

			m_pendingTicketList->erase(iterPendingTicketList);
			break;
		}
		else if (   pendingTicket.m_chatServerChatLogReceived
		         && pendingTicket.m_gameServerChatLogReceived)
		{
			// If the ticket has populated its 2 chat log fields, if so, send it out!

			addChatLogs(sortedChatLog, pendingTicket.m_chatServerChatLog);
			addChatLogs(sortedChatLog, pendingTicket.m_gameServerChatLog);

			Unicode::String report;
			createReport(pendingTicket, sortedChatLog, report);

			Unicode::String hiddenDetails(pendingTicket.m_hiddenDetails);
			hiddenDetails.append(Unicode::narrowToWide("\n\n"));
			hiddenDetails.append(report);

			LOG("CSServer", ("processPendingTickets() All chat logs received"));
			m_csInterface.decrementPendingTicketCount(pendingTicket.m_networkId);

			bool const logChat = false;
			createTicket(pendingTicket.m_networkId, pendingTicket.m_ticket, hiddenDetails, Unicode::emptyString, pendingTicket.m_suid, logChat);

			// Bail, we will catch another ticket next frame

			m_pendingTicketList->erase(iterPendingTicketList);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::addChatLogs(SortedChatLog &sortedChatLog, std::vector<ChatLogEntry> &chatLogs)
{
	std::vector<ChatLogEntry>::const_iterator iterLogEntries = chatLogs.begin();
	Unicode::String chatMessage;

	for (; iterLogEntries != chatLogs.end(); ++iterLogEntries)
	{
		ChatLogEntry const &chatLogEntry = (*iterLogEntries);

		ChatLogManager::buildLogEntry(chatMessage, chatLogEntry.m_from, chatLogEntry.m_to, chatLogEntry.m_message, chatLogEntry.m_channel, chatLogEntry.m_time);

		sortedChatLog.insert(std::make_pair(chatLogEntry.m_time, chatMessage));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::createReport(PendingTicket const & pendingTicket, SortedChatLog const & sortedChatLog, Unicode::String & report)
{
	// Put the final report together

	Unicode::String header;
	std::string const reportingPlayer(Unicode::wideToNarrow(pendingTicket.m_ticket.character));
	std::string const harassingPlayer(Unicode::wideToNarrow(pendingTicket.m_harassingPlayerName));

	ChatLogManager::getReportHeader(header, reportingPlayer, pendingTicket.m_networkId, std::string(), 0, harassingPlayer, NetworkId::cms_invalid, std::string(), 0);

	report.clear();
	report.append(header);

	if (!pendingTicket.m_chatServerChatLogReceived)
	{
		report.append(Unicode::narrowToWide("Unable to retrieve the ChatServer chat logs.\n"));
	}

	if (!pendingTicket.m_gameServerChatLogReceived)
	{
		report.append(Unicode::narrowToWide("Unable to retrieve the GameServer spatial chat logs.\n"));
	}

	FormattedString<8192> fs;
	SortedChatLog::const_iterator iterSortedChatLog = sortedChatLog.begin();
	int count = 0;
	
	for (; iterSortedChatLog != sortedChatLog.end(); ++iterSortedChatLog)
	{
		char const * const text = fs.sprintf("[%3d]", ++count);

		report += Unicode::narrowToWide(text);
		report += iterSortedChatLog->second;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceServer::setDone(bool done)
{
	m_done = done;
}

//-----------------------------------------------------------------------

