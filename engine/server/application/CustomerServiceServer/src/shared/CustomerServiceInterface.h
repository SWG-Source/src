// CustomerServiceInterface.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef _INCLUDED_CustomerServiceInterface_H
#define _INCLUDED_CustomerServiceInterface_H

#include "CSAssistgameapi.h"
#include "CSAssistgameobjects.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"
#include <vector>

extern "C" {
#include "libxml/parser.h"
}

using namespace CSAssist;

class ConnectionServerConnection;
class GameNetworkMessage;

//--------------------------------------------------------

class CustomerServiceInterface : public CSAssistGameAPI
{
private:

	class ClientInfo
	{
	public:

		ClientInfo();

		ConnectionServerConnection *m_connection;
		unsigned int                m_stationUserId;
		int                         m_ticketCount;
		int                         m_pendingTicketCount;
	};

	typedef stdunordered_map<NetworkId, ClientInfo, NetworkId::Hash>::fwd ClientConnectionMap;
	typedef stdmap<unsigned int, NetworkId>::fwd                     SuidToNetworkIdMap;
	typedef stdvector<CustomerServiceCategory>::fwd                  CategoryList;

	CategoryList * const m_englishCategoryList;
	CSAssistGameAPITrack m_englishCategoryTrack;

	CategoryList * const m_japaneseCategoryList;
	CSAssistGameAPITrack m_japaneseCategoryTrack;

	ClientConnectionMap *       m_clientConnectionMap;
	SuidToNetworkIdMap *        m_suidToNetworkIdMap;
	bool                        m_connectionToBackEndEstablised;

protected:

	void parseIssueChild(CategoryList & categoryList, xmlNodePtr childPtr);
	void parseIssueHierarchy(CategoryList & categoryList, Unicode::String const & xmlData);

public:
	CustomerServiceInterface(const char *server, const unsigned port, const unsigned timeout);
	virtual ~CustomerServiceInterface();

	void                addPlayer(NetworkId const &networkId, ConnectionServerConnection *connection, const unsigned int suid);
	void                removePlayer(NetworkId const &networkId);
	void                checkQueuedLogins();
	void                sendToClient(const NetworkId &player, const GameNetworkMessage &message);
	bool                getNetworkId(unsigned int suid, NetworkId &networkId);
	bool                getSuid(NetworkId const &networkId, unsigned int &suid);
	int                 getTicketCount(NetworkId const &networkId) const;
	void                connectionServerConnectionDestroyed(ConnectionServerConnection *);
	void                requestUnRegisterCharacter(const NetworkId &requester);
	void                incrementPendingTicketCount(NetworkId const &networkId);
	void                decrementPendingTicketCount(NetworkId const &networkId);
	void                incrementTicketCount(NetworkId const &networkId);
	void                decrementTicketCount(NetworkId const &networkId);

	CategoryList const & getEnglishCategoryList() const;
	CategoryList const & getJapaneseCategoryList() const;

	virtual void OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnConnectRejectedCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyData);
	virtual void OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *searchResults, const unsigned numberRead);
	virtual void OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody, const CSAssistUnicodeChar *articleTitle);
	virtual void OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned numberReturned, const unsigned totalNumber, const CSAssistGameAPITicket *ticketBody);
	virtual void OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned int numberRead);
	virtual void OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);
	virtual void OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets);
	virtual void OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnTicketChange(const CSAssistGameAPITicketID ticketId, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	virtual void OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);

private:

	CustomerServiceInterface & operator =(CustomerServiceInterface const &);
};

#endif //_INCLUDED_CustomerServiceInterface_H
