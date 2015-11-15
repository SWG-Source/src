// CustomerServiceServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef _INCLUDED_CustomerServiceServer_H
#define _INCLUDED_CustomerServiceServer_H

#include "CustomerServiceInterface.h" 

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"

//--------------------------------------------------------
namespace MessageDispatch
{
	class Callback;
}

class CentralServerConnection; 
class ConnectionServerConnection;
class EnumerateServers; 
class Service;

//--------------------------------------------------------

class CustomerServiceServer
{
private:

	//Methods

	void    onEnumerateServers(const EnumerateServers &servers);
	void    update();
	void    processPendingTickets();

	//typedefs

	typedef stdset<ConnectionServerConnection *>::fwd ConnectionServerSet;
	typedef std::multimap<unsigned long, Unicode::String> SortedChatLog;

	//Member variables

	MessageDispatch::Callback *   m_callback;     
	CentralServerConnection *     m_centralServerConnection;
	ConnectionServerSet *         m_connectionServerSet;
	bool                          m_done;
	CustomerServiceInterface      m_csInterface;
	static CustomerServiceServer *m_instance;
	Service *                     m_gameServerService;
	Service *                     m_chatServerService;
	unsigned int                  m_nextSequenceId;

	struct PendingTicket
	{
		PendingTicket();

		NetworkId                 m_networkId;
		unsigned long             m_time;
		CSAssistGameAPITicket     m_ticket;
		Unicode::String           m_hiddenDetails;
		Unicode::String           m_harassingPlayerName;
		unsigned int              m_suid;
		std::vector<ChatLogEntry> m_chatServerChatLog;
		bool                      m_chatServerChatLogReceived;
		std::vector<ChatLogEntry> m_gameServerChatLog;
		bool                      m_gameServerChatLogReceived;
	};

	typedef stdmap<unsigned int, PendingTicket>::fwd  PendingTicketList;
	PendingTicketList *m_pendingTicketList;

	void createTicket(NetworkId const &networkId, const CSAssistGameAPITicket &ticket, Unicode::String const &hiddenDetails, Unicode::String const &harassingPlayerName, const unsigned int suid, const bool logChat);
	void addChatLogs(SortedChatLog &sortedChatLog, std::vector<ChatLogEntry> &chatLogs);
	void createReport(PendingTicket const &pendingTicket, SortedChatLog const &sortedChatLog, Unicode::String & report);

public:
	CustomerServiceServer();
	~CustomerServiceServer();

	static CustomerServiceServer & getInstance();

	Service *getGameServerService() const;
	Service *getChatServerService() const;

	void run();
	bool isDone() {return m_done;}
	void reconnectToCentralServer();
	void handleChatServerChatLog(ChatOnRequestLog const &chatOnRequestLog);
	void handleGameServerChatLog(ChatOnRequestLog const &chatOnRequestLog);

	void requestRegisterCharacter(const NetworkId &requester, ConnectionServerConnection *connection, const unsigned int suid);
	void requestUnRegisterCharacter(const NetworkId &requester);
	void appendComment(const NetworkId &requester, unsigned int suid, const std::string &characterName, unsigned int ticketId, const Unicode::String &comment);
	void cancelTicket(const NetworkId &requester, unsigned int suid, unsigned int ticketId, const Unicode::String &comment);
	void createTicket(const NetworkId &requester, unsigned int suid, const std::string &characterName, unsigned int category, unsigned int subCategory, const Unicode::String &details, const Unicode::String &hiddenDetails, Unicode::String const &harassingPlayerName, const std::string &language, bool isBug);
	void getArticle(const NetworkId &requester, const std::string &id, const std::string &language);
	void getComments(const NetworkId &requester, unsigned int ticketId);
	void getTickets(const NetworkId &requester, unsigned int suid);
	void requestCategories(NetworkId const & requester, std::string const & language);
	void searchKnowledgeBase(const NetworkId &requester, const Unicode::String &searchString, const std::string &language);
	void connectionServerConnectionDestroyed(ConnectionServerConnection *connectionServerConnection);
	void requestNewTicketActivity(const NetworkId &requester, unsigned int suid);
	void setDone(bool done);

private:

	// Disabled

	CustomerServiceServer & operator =(CustomerServiceServer const &);
};

#endif //_INCLUDED_CustomerServiceServer_H
