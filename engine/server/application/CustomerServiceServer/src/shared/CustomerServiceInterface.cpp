///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceInterface.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include "FirstCustomerServiceServer.h"
#include "CustomerServiceInterface.h"
#include "CustomerServiceServer.h"
#include "UnicodeUtils.h"

#include "ConfigCustomerServiceServer.h"
#include "ConnectionServerConnection.h"

#include "sharedFoundation/Clock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/AppendCommentResponseMessage.h"
#include "sharedNetworkMessages/CancelTicketResponseMessage.h"
#include "sharedNetworkMessages/ConnectPlayerResponseMessage.h"
#include "sharedNetworkMessages/CreateTicketResponseMessage.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GetArticleResponseMessage.h"
#include "sharedNetworkMessages/GetCommentsResponseMessage.h"
#include "sharedNetworkMessages/GetTicketsResponseMessage.h"
#include "sharedNetworkMessages/NewTicketActivityResponseMessage.h"
#include "sharedNetworkMessages/SearchKnowledgeBaseResponseMessage.h"

#include <map>

using namespace CSAssist;

///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceInterface::ClientInfo
//
///////////////////////////////////////////////////////////////////////////////

CustomerServiceInterface::ClientInfo::ClientInfo()
 : m_connection(nullptr)
 , m_stationUserId(0)
 , m_ticketCount(-1)
 , m_pendingTicketCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// CustomerServiceInterface
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

CustomerServiceInterface::CustomerServiceInterface(const char *server, const unsigned port, const unsigned timeout) :
CSAssistGameAPI(server, port, timeout, CSASSIST_APIFLAG_ASSUME_RECONNECT),
m_englishCategoryList(new CategoryList),
m_japaneseCategoryList(new CategoryList),
m_clientConnectionMap(new ClientConnectionMap),
m_suidToNetworkIdMap(new SuidToNetworkIdMap),
m_connectionToBackEndEstablised(false)
{
}

//-----------------------------------------------------------------------

CustomerServiceInterface::~CustomerServiceInterface()
{
	delete m_englishCategoryList;
	delete m_japaneseCategoryList;

	delete m_clientConnectionMap;
	m_clientConnectionMap = nullptr;

	delete m_suidToNetworkIdMap;
	m_suidToNetworkIdMap = nullptr;
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnConnectCSAssist(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result,
	const void * /*userData*/
)
{
	char const * const text = FormattedString<1024>().sprintf("OnConnectCSAssist() track(%i) result(%i) (%s)", track, result, getErrorString(result));
	REPORT_LOG(true, ("CustomerServiceInterface::%s\n", text));
	LOG("CSServer", ("%s", text));

	if (result == CSASSIST_RESULT_SUCCESS)
	{
		if (m_connectionToBackEndEstablised)
		{
			LOG("CSServer", ("OnConnectCSAssist() - Reconnecting to the CS back-end, connection was lost."));
		}

		m_connectionToBackEndEstablised = true;
		m_englishCategoryTrack = requestGetIssueHierarchy(nullptr, Unicode::narrowToWide("Default").data(), Unicode::narrowToWide("en").data());;
		m_japaneseCategoryTrack = requestGetIssueHierarchy(nullptr, Unicode::narrowToWide("Default").data(), Unicode::narrowToWide("ja").data());;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnConnectRejectedCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void * /*userData*/)
{
	m_connectionToBackEndEstablised = false;

	char const * const text = FormattedString<1024>().sprintf("OnConnectRejectedCSAssist() track(%i) result(%i) (%s)", track, result, getErrorString(result));
	REPORT_LOG(true, ("CustomerServiceInterface::%s\n", text));
	LOG("CSServer", ("%s", text));
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::parseIssueChild(CategoryList & categoryList, xmlNodePtr childPtr)
{
	static CustomerServiceCategory *currentCategory = nullptr;
	xmlNodePtr child = childPtr->children;

	//start element
	const char *tagName = reinterpret_cast<const char *>(childPtr->name);

	Unicode::String categoryName;
	int categoryId = 0;
	char * val = reinterpret_cast<char *>(xmlGetProp(childPtr, (const unsigned char *)"name"));

	if (val)
	{
		categoryName = Unicode::utf8ToWide(val);
	}

	val = reinterpret_cast<char *>(xmlGetProp(childPtr, (const unsigned char *)"id")); 
	if (val)
	{
		categoryId = atoi(val);
	}

	// Check for bug type
	
	val = reinterpret_cast<char *>(xmlGetProp(childPtr, (const unsigned char *)"isBugType")); 
	
	bool const isBugType = (val != nullptr) && (strcmp(val, "true") == 0);
	
	// Check for service type
	
	val = reinterpret_cast<char *>(xmlGetProp(childPtr, (const unsigned char *)"isServiceType")); 
	
	bool const isServiceType = (val != nullptr) && (strcmp(val, "true") == 0);
	
	// Check if valid

	val = reinterpret_cast<char *>(xmlGetProp(childPtr, (const unsigned char *)"invalid")); 

	bool const invalid = (val != nullptr) && (strcmp(val, "true") == 0);

	if (!invalid)
	{
		if (strcmp(tagName, "category") == 0)
		{
			CustomerServiceCategory * const category = new CustomerServiceCategory(categoryName, categoryId, isBugType, isServiceType);
			currentCategory = category; 
		}
		else if (strcmp(tagName, "subCategory") == 0)
		{
			CustomerServiceCategory const category(categoryName, categoryId, isBugType, isServiceType);

			if (currentCategory)
			{
				currentCategory->addSubCategory(category);
			}
		}
	}
	else
	{
		// Skip these categories

		if (strcmp(tagName, "category") == 0)
		{
			LOG("CSServer", ("parseIssueChild() Skipping invalid category(%s) id(%d)", categoryName.c_str(), categoryId));
		}
		else if (strcmp(tagName, "subCategory") == 0)
		{
			LOG("CSServer", ("parseIssueChild() Skipping invalid sub-category(%s) id(%d)", categoryName.c_str(), categoryId));
		}
	}

	while (child != nullptr)
	{
		if (child->name != nullptr)
		{
			parseIssueChild(categoryList, child);
		}
		child = child->next;
	}

	if (   !invalid
	    && (strcmp(tagName, "category") == 0))
	{
		categoryList.push_back(*currentCategory);

		if (currentCategory)
		{
			delete currentCategory;
			currentCategory = nullptr;
		}
	}

	//end element
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::parseIssueHierarchy(CategoryList & categoryList, Unicode::String const & xmlData)
{
	xmlDocPtr xmlInfo = nullptr;
	Unicode::UTF8String xmlDataUtf8(Unicode::wideToUTF8(xmlData));
	
	if ((xmlInfo = xmlParseMemory(xmlDataUtf8.c_str(), xmlDataUtf8.length())) != nullptr)
	{
		xmlNodePtr xmlCurrent = xmlDocGetRootElement(xmlInfo);
		if (xmlCurrent != nullptr)
		{
			parseIssueChild(categoryList, xmlCurrent);
		}
		xmlFreeDoc(xmlInfo);
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnGetIssueHierarchy(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void * /*userData*/, 
	const CSAssistGameAPIXML hierarchyBody, 
	const unsigned modifyData
)
{
	LOG("CSServer", ("OnGetIssueHierarchy() track(%i) modifyData(%u) result(%i:%s)", track, modifyData, result, getErrorString(result)));

	if (result != 0)
	{
		LOG("CSServer", ("ERROR: failure code returned...the hierarchy will not be parsed."));
		return;
	}

	if (hierarchyBody != nullptr)
	{
		if (track == m_englishCategoryTrack)
		{
			m_englishCategoryList->clear();
			parseIssueHierarchy(*m_englishCategoryList, hierarchyBody);
		}
		else if (track == m_japaneseCategoryTrack)
		{
			m_japaneseCategoryList->clear();
			parseIssueHierarchy(*m_japaneseCategoryList, hierarchyBody);
		}
		else
		{
			LOG("CSServer", ("ERROR: Unknown track specified(%u)...the hierarchy will not be parsed.", track));
		}
	}
	else
	{
		LOG("CSServer", ("ERROR: nullptr hierarchy returned from platform...why did this happen?", track, modifyData, result, getErrorString(result)));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnCreateTicket(
	const CSAssistGameAPITrack track,
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const CSAssistGameAPITicketID ticket
)	
{
	CreateTicketResponseMessage message(result, ticket);
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnCreateTicket() track(%i) result(%i) (%s) networkId(%s) ticketId(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", ticket));

	if (tmpNetworkId != nullptr)
	{
		if (result == CSASSIST_RESULT_SUCCESS)
		{
			incrementTicketCount(*tmpNetworkId);
		}

		decrementPendingTicketCount(*tmpNetworkId);

		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnAppendTicketComment(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result,
	const void *userData, 
	const CSAssistGameAPITicketID ticket
)	
{
	AppendCommentResponseMessage message(result, ticket);
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnAppendTicketComment() track(%i) result(%i) (%s) networkId(%s) ticketId(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", ticket));

	if (tmpNetworkId != nullptr)
	{
		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnCancelTicket(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const CSAssistGameAPITicketID ticket
)	
{
	CancelTicketResponseMessage message(result, ticket);
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnCancelTicket() track(%i) result(%i) (%s) networkId(%s) ticketId(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", ticket));

	if (tmpNetworkId != nullptr)
	{
		if (result == CSASSIST_RESULT_SUCCESS)
		{
			decrementTicketCount(*tmpNetworkId);
		}

		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnGetTicketByCharacter(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const unsigned numberReturned, 
	const unsigned totalNumber, 
	const CSAssistGameAPITicket *ticketBody
)
{
	std::vector<CustomerServiceTicket> tickets;
	for (unsigned int i = 0; i < numberReturned; i++)
	{
		tickets.push_back(CustomerServiceTicket(ticketBody[i].category[0], 
			ticketBody[i].category[1], 
			std::string(Unicode::wideToNarrow(ticketBody[i].character).c_str()),
			Unicode::String(ticketBody[i].details),
			Unicode::wideToNarrow(ticketBody[i].language), 
			ticketBody[i].ticketID, 
			ticketBody[i].modifiedDate,
			ticketBody[i].isRead,
			(ticketBody[i].status == TICKET_STATUS_PENDING_CLOSED) || (ticketBody[i].status == TICKET_STATUS_CLOSED)));
	}
	GetTicketsResponseMessage message(result, totalNumber, tickets); 	
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnGetTicketByCharacter() track(%i) result(%i) (%s) networkId(%s) totalNumber(%i) numberReturned(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", totalNumber, numberReturned));

	if (tmpNetworkId != nullptr)
	{
		// Save the number of tickets this player has

		ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(*tmpNetworkId);

		if (iterClientMap != m_clientConnectionMap->end())
		{
			iterClientMap->second.m_ticketCount = static_cast<int>(totalNumber);
		}

		// Send the tickets to the client

		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnGetTicketComments(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const CSAssistGameAPITicketComment *comments, 
	const unsigned int numberRead
)
{
	std::vector<CustomerServiceComment> commentsVector;
	for (unsigned int i = 0; i < numberRead; i++)
	{
		commentsVector.push_back(CustomerServiceComment(comments[i].ticketID, 
			comments[i].commentID, 
			comments[i].type == CSASSIST_COMMENT_FROM_CSR,
			Unicode::String(comments[i].comment),
			Unicode::wideToNarrow(comments[i].name)));
	}
	GetCommentsResponseMessage message(result, commentsVector); 	

	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnGetTicketComments() track(%i) result(%i) (%s) networkId(%s) numberRead(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", numberRead));

	if (tmpNetworkId != nullptr)
	{
		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnSearchKB(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const CSAssistGameAPISearchResult *searchResults, 
	const unsigned numberRead
)
{
	std::vector<CustomerServiceSearchResult> searchResultsVector;
	for (unsigned int i = 0; i < numberRead; i++)
	{
		searchResultsVector.push_back(CustomerServiceSearchResult(
			Unicode::String(searchResults[i].title), 
			Unicode::wideToNarrow(searchResults[i].idstring), 
			searchResults[i].matchPercent));
	}
	SearchKnowledgeBaseResponseMessage message(result, searchResultsVector); 	
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnSearchKB() track(%i) result(%i) (%s) networkId(%s) numberRead(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", numberRead));

	if (tmpNetworkId != nullptr)
	{
		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnGetKBArticle(
	const CSAssistGameAPITrack track, 
	const CSAssistGameAPIResult result, 
	const void *userData, 
	const CSAssistGameAPIXML articleBody,
	const CSAssistUnicodeChar* 
)
{
	LOG("CSServer", ("OnGetKBArticle() track(%i) result(%i) (%s)", track, result, getErrorString(result)));

	if(articleBody)
	{
		GetArticleResponseMessage message(result, Unicode::String(articleBody));
		const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);
		if (tmpNetworkId != nullptr)
		{
			sendToClient(*tmpNetworkId, message);

			delete tmpNetworkId;
			tmpNetworkId = nullptr;
		}
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnIssueHierarchyChanged(
	const CSAssistUnicodeChar *version, 
	const CSAssistUnicodeChar *language
)
{
	LOG("CSServer", ("OnIssueHierarchyChanged()"));

	requestGetIssueHierarchy(nullptr, version, language);
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets)
{
	NewTicketActivityResponseMessage message(static_cast<bool>(NewActivityFlag), HasTickets);

	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnNewTicketActivity() track(%i) result(%i) (%s) networkId(%s) activity(%s) ticket count(%i)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA", NewActivityFlag ? "yes" : "no", HasTickets));

	if (tmpNetworkId != nullptr)
	{
		// Save the number of tickets this player has

		ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(*tmpNetworkId);

		if (iterClientMap != m_clientConnectionMap->end())
		{
			iterClientMap->second.m_ticketCount = static_cast<int>(HasTickets);
		}

		// Send the new ticket activity to the client

		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
{
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnMarkTicketRead() track(%i) result(%i) (%s) networkId(%s)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA"));

	if (tmpNetworkId != nullptr)
	{
		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnTicketChange(const CSAssistGameAPITicketID ticketId, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
{
	UNREF(character);

	NetworkId networkId;
	
	if (getNetworkId(uid, networkId))
	{
		LOG("CSServer", ("OnTicketChange() ticketId(%i) stationId(%i) networkId(%s)", ticketId, uid, networkId.getValueString().c_str()));

		CustomerServiceServer::getInstance().getTickets(networkId, uid);
	}
	else
	{
		LOG("CSServer", ("OnTicketChange() Unable to locate player: ticketId(%i) stationId(%i) networkId: NA", ticketId, uid));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
{
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnRegisterCharacter() track(%i) result(%i) (%s) networkId(%s)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA"));

	if (tmpNetworkId != nullptr)
	{
		ConnectPlayerResponseMessage message(result);
		
		sendToClient(*tmpNetworkId, message);

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
{
	const NetworkId *tmpNetworkId = reinterpret_cast<const NetworkId *>(userData);

	LOG("CSServer", ("OnUnRegisterCharacter() track(%i) result(%i) (%s) networkId(%s)", track, result, getErrorString(result), (tmpNetworkId != nullptr) ? tmpNetworkId->getValueString().c_str() : "NA"));

	if (tmpNetworkId != nullptr)
	{
		// If the player was successfully unregistered, remove the local cached reference

		if (result == CSASSIST_RESULT_SUCCESS)
		{
			removePlayer(*tmpNetworkId);
		}

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::addPlayer(NetworkId const &networkId, ConnectionServerConnection *connection, const unsigned int suid)
{
	LOG("CSServer", ("addPlayer() networkId(%s) suid(%i)", networkId.getValueString().c_str(), suid));

	ClientConnectionMap::iterator iterClientConnectionMap = m_clientConnectionMap->find(networkId);

	if (iterClientConnectionMap == m_clientConnectionMap->end())
	{
		ClientInfo clientInfo;
		clientInfo.m_connection = connection;
		clientInfo.m_stationUserId = suid;

		IGNORE_RETURN(m_clientConnectionMap->insert(std::make_pair(networkId, clientInfo)));
		IGNORE_RETURN(m_suidToNetworkIdMap->insert(std::make_pair(suid, networkId)));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::removePlayer(NetworkId const &networkId)
{
	ClientConnectionMap::iterator cf = m_clientConnectionMap->find(networkId);

	if (cf != m_clientConnectionMap->end())
	{
		LOG("CSServer", ("removePlayer() networkId(%s)", networkId.getValueString().c_str()));

		// Remove the suid id map item

		unsigned int const suid = cf->second.m_stationUserId;

		SuidToNetworkIdMap::iterator iterStationUserIdToNetworkIdMap = m_suidToNetworkIdMap->find(suid);

		if (iterStationUserIdToNetworkIdMap != m_suidToNetworkIdMap->end())
		{
			m_suidToNetworkIdMap->erase(iterStationUserIdToNetworkIdMap);
		}

		m_clientConnectionMap->erase(cf);
	}
	else
	{
		LOG("CSServer", ("removePlayer() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

bool CustomerServiceInterface::getNetworkId(unsigned int suid, NetworkId &networkId)
{
	bool result = false;
	SuidToNetworkIdMap::const_iterator iterStationUserIdToNetworkIdMap = m_suidToNetworkIdMap->find(suid);

	if (iterStationUserIdToNetworkIdMap != m_suidToNetworkIdMap->end())
	{
		result = true;
		networkId = iterStationUserIdToNetworkIdMap->second;
	}

	return result;
}

//-----------------------------------------------------------------------

bool CustomerServiceInterface::getSuid(NetworkId const &networkId, unsigned int &suid)
{
	bool result = false;
	ClientConnectionMap::const_iterator cf = m_clientConnectionMap->find(networkId);

	if (cf != m_clientConnectionMap->end())
	{
		result = true;
		suid = cf->second.m_stationUserId;
	}

	return result;
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::sendToClient(const NetworkId &player, const GameNetworkMessage &message)
{
	ClientConnectionMap::iterator cf = m_clientConnectionMap->find(player);

	if (cf != m_clientConnectionMap->end())
	{
		ConnectionServerConnection *connection = cf->second.m_connection;

		if (connection)
		{
			LOG("CSServer", ("sendToClient() networkId(%s) cmdName(%s)", player.getValueString().c_str(), message.getCmdName().c_str()));

			connection->sendToClient(player, message);
		}
		else
		{
			LOG("CSServer", ("sendToClient() Connection is nullptr: networkId(%s) cmdName(%s)", player.getValueString().c_str(), message.getCmdName().c_str()));
		}
	}
	else
	{
		LOG("CSServer", ("sendToClient() Unable to locate player: networkId(%s) cmdName(%s)", player.getValueString().c_str(), message.getCmdName().c_str()));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::connectionServerConnectionDestroyed(ConnectionServerConnection * conn)
{
	ClientConnectionMap::iterator i;

	for(i = m_clientConnectionMap->begin(); i != m_clientConnectionMap->end(); ++i)
	{
		if(i->second.m_connection == conn)
		{
			requestUnRegisterCharacter((*i).first);
		}
	}
}

//-----------------------------------------------------------------------

CustomerServiceInterface::CategoryList const & CustomerServiceInterface::getEnglishCategoryList() const
{
	return *m_englishCategoryList;
}

//-----------------------------------------------------------------------

CustomerServiceInterface::CategoryList const & CustomerServiceInterface::getJapaneseCategoryList() const
{
	return *m_japaneseCategoryList;
}

//-----------------------------------------------------------------------

int CustomerServiceInterface::getTicketCount(NetworkId const &networkId) const
{
	int result = -1;

	// Get the sum of the current tickets and pending tickets

	ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(networkId);

	if (iterClientMap != m_clientConnectionMap->end())
	{
		result = (iterClientMap->second.m_ticketCount + iterClientMap->second.m_pendingTicketCount);
	}
	else
	{
		LOG("CSServer", ("getTicketCount() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}

	return result;
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::requestUnRegisterCharacter(const NetworkId &requester)
{
	unsigned int suid = 0;

	if (getSuid(requester, suid))
	{
		LOG("CSServer", ("requestUnRegisterCharacter() networkId(%s) suid(%i)", requester.getValueString().c_str(), suid));

		NetworkId *tmpNetworkId = new NetworkId(requester);
		CSAssistGameAPI::requestUnRegisterCharacter(reinterpret_cast<const void *>(tmpNetworkId), suid, nullptr);
	}
	else
	{
		LOG("CSServer", ("requestUnRegisterCharacter() Unable to locate player for unregister: networkId(%s) suid: NA", requester.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::incrementPendingTicketCount(NetworkId const &networkId)
{
	ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(networkId);

	if (iterClientMap != m_clientConnectionMap->end())
	{
		++(iterClientMap->second.m_pendingTicketCount);

		LOG("CSServer", ("incrementPendingTicketCount() Increasing (by one) the pending ticket count of networkId(%s) to tickets(%d) pending(%d).", networkId.getValueString().c_str(), iterClientMap->second.m_ticketCount, iterClientMap->second.m_pendingTicketCount));
	}
	else
	{
		LOG("CSServer", ("incrementPendingTicketCount() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::decrementPendingTicketCount(NetworkId const &networkId)
{
	// Decrement the player's ticket count

	ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(networkId);

	if (iterClientMap != m_clientConnectionMap->end())
	{
		--(iterClientMap->second.m_pendingTicketCount);

		LOG("CSServer", ("decrementPendingTicketCount() Decreasing (by one) the pending ticket count of networkId(%s) to tickets(%d) pending(%d).", networkId.getValueString().c_str(), iterClientMap->second.m_ticketCount, iterClientMap->second.m_pendingTicketCount));
	}
	else
	{
		LOG("CSServer", ("decrementPendingTicketCount() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::incrementTicketCount(NetworkId const &networkId)
{
	ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(networkId);

	if (iterClientMap != m_clientConnectionMap->end())
	{
		++(iterClientMap->second.m_ticketCount);

		LOG("CSServer", ("incrementTicketCount() Increasing (by one) the ticket count of networkId(%s) to tickets(%d) pending(%d).", networkId.getValueString().c_str(), iterClientMap->second.m_ticketCount, iterClientMap->second.m_pendingTicketCount));
	}
	else
	{
		LOG("CSServer", ("incrementTicketCount() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

void CustomerServiceInterface::decrementTicketCount(NetworkId const &networkId)
{
	// Decrement the player's ticket count

	ClientConnectionMap::iterator iterClientMap = m_clientConnectionMap->find(networkId);

	if (iterClientMap != m_clientConnectionMap->end())
	{
		--(iterClientMap->second.m_ticketCount);

		LOG("CSServer", ("decrementTicketCount() Decreasing (by one) the ticket count of networkId(%s) to tickets(%d) pending(%d).", networkId.getValueString().c_str(), iterClientMap->second.m_ticketCount, iterClientMap->second.m_pendingTicketCount));
	}
	else
	{
		LOG("CSServer", ("decrementTicketCount() Unable to locate player: networkId(%s)", networkId.getValueString().c_str()));
	}
}

///////////////////////////////////////////////////////////////////////////////
