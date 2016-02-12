#ifndef __CSASSISTGAMEAPICORE_H
#define __CSASSISTGAMEAPICORE_H

//------------------------------------------------------------------------------
//
// CSAssistgameapicore.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Main API internal header file
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>

#ifdef USE_UDP_LIBRARY
#include "UdpLibrary/UdpLibrary.hpp"
#else
#include <TcpLibrary/TcpConnection.h>
#include <TcpLibrary/TcpHandlers.h>
#include <TcpLibrary/TcpManager.h>
#endif

#include "Base/Archive.h"

#include "CSAssistgameapi.h"
#include "CSAssistgameobjects.h"
#include "packdata.h"
#include "Unicode/Unicode.h"

namespace CSAssist
{

static const unsigned MAX_PACKETS_PER_SEC	= 500;
static const unsigned NORMAL_TIMEOUT_VAL	= 60;	 // 60 seconds
//static const unsigned NORMAL_TIMEOUT_VAL	= 5;		 // debug = 5 seconds
static const unsigned TRACK_CONNECTLB 		= 0;
static const unsigned CONNECTLB_TIMEOUT		= 5;	// 5 seconds

struct timeout {
	inline timeout()
	{
		time = 0;
		track = 0;
	}
	inline timeout(unsigned tr, int ti)
	{
		time = ti;
		track = tr;
	}

	int time;
	unsigned track;
};

enum CONNECT_STATE
{
	CONNECT_INIT = 0
	,CONNECT_REDIRECT_INIT					// 1
	,CONNECT_REDIRECT_NEGOTIATING			// 2
	,CONNECT_REDIRECT_CONNECTED				// 3
	,CONNECT_REDIRECT_DISCONNECTED			// 4
	,CONNECT_BACKEND_INIT					// 5
	,CONNECT_BACKEND_NEGOTIATING			// 6
	,CONNECT_BACKEND_CONNECTED				// 7
	,CONNECT_BACKEND_CONNECTED_AND_AUTHED	// 8
	,CONNECT_BACKEND_DISCONNECTED			// 9
};

struct ServerID {
	std::string host;
	unsigned port;
};

//--------------------------------------
class RegisteredCharacter
//--------------------------------------
{
public:

	RegisteredCharacter(const unsigned uid, const Plat_Unicode::String character, const unsigned avaconID);
	virtual ~RegisteredCharacter();

	Plat_Unicode::String getCharacter()		{ return character; }
	unsigned getUID()						{ return uid; }
	unsigned getAvaconID()					{ return avaconID; }

private:
	CSAssistGameAPIUID		uid;			// station UID
	Plat_Unicode::String	character;		// character name
	unsigned				avaconID;		// universal chat system UID
};

class Response;
class Request;
class CSAssistReceiver;
class CSAssistGameAPI;
class RegisteredCharacter;

//--------------------------------------
class CSAssistGameAPIcore
//--------------------------------------
{
public:
	friend class CSAssist::CSAssistReceiver;

	CSAssistGameAPIcore(CSAssistGameAPI *api, const char *host, const unsigned port, const unsigned timeout, const unsigned flags);
	CSAssistGameAPIcore(CSAssistGameAPI *api, const char *serverList, const unsigned timeout, const unsigned flags);
	virtual ~CSAssistGameAPIcore();

	// **** ADD CODE HERE ****
	CSAssistGameAPITrack connectCSAssist(const void *userData, const CSAssistUnicodeChar *GameName, const CSAssistUnicodeChar *ServerName);
	CSAssistGameAPITrack disconnectCSAssist(const void *userData);
	CSAssistGameAPITrack requestNewTicketActivity(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	CSAssistGameAPITrack requestRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned avaconID);
	CSAssistGameAPITrack requestUnRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	CSAssistGameAPITrack requestGetIssueHierarchy(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);
	CSAssistGameAPITrack requestCreateTicket(const void *userData, const CSAssistGameAPITicket *ticketBody, const CSAssistUnicodeChar *XMLBody, const CSAssistGameAPIUID uid);
	CSAssistGameAPITrack requestAppendTicketComment(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistUnicodeChar *comment);
	CSAssistGameAPITrack requestGetTicketByID(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned FlagAsRead);
	CSAssistGameAPITrack requestGetTicketComments(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned Start, const unsigned Count, const CSAssistGameAPIOffset offset);
	CSAssistGameAPITrack requestGetTicketByCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned Start, const unsigned Count, const unsigned FlagAsRead);
	CSAssistGameAPITrack requestMarkTicketRead(const void *userData, const CSAssistGameAPITicketID ticket);
	CSAssistGameAPITrack requestCancelTicket(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *comment);
	CSAssistGameAPITrack requestGetTicketCommentsCount(const void *userData, const CSAssistGameAPITicketID ticket);
	CSAssistGameAPITrack replyGameLocation(const void *userData, const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID csruid, const CSAssistUnicodeChar *locationString);
	CSAssistGameAPITrack requestGetDocumentList(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);
	CSAssistGameAPITrack requestGetDocument(const void *userData, const unsigned id);
	CSAssistGameAPITrack requestGetTicketXMLBlock(const void *userData, const CSAssistGameAPITicketID id);
	CSAssistGameAPITrack requestGetKBArticle(const void *userData, const CSAssistUnicodeChar *id, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid);
	CSAssistGameAPITrack requestSearchKB(const void *userData, const CSAssistUnicodeChar *searchstring, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid);
	void				 setMaxPacketsPerSecond(const unsigned n);

	void Update();
	void CSAssistGameCallback(Response *);
	void localUnRegisterCharacter(const unsigned uid, const Plat_Unicode::String character);
	void localRegisterCharacter(const unsigned uid, const Plat_Unicode::String character, const unsigned avaconID);
	void reregisterCharacters();
	bool isInternalTrackingNumber(const unsigned track);
	void batchRegisterCharacters(const unsigned numToSend);
	const char* GetConnectedHost();// { return m_ip.c_str(); }
	int	 GetConnectedPort();// { return m_port; }

private:
	void SubmitRequest(Request *req, Response *res);
	void SubmitImmediateError(Response *res);
	Response *getPending(CSAssistGameAPITrack track);
	Response *getResponse(CSAssistGameAPITrack track);
	Response *createServerResponse(short msgtype);
	void SubmitRequestInt(Request *req, Response *res);
	Response *getPendingInt(CSAssistGameAPITrack track);
	Response *getResponseInt(CSAssistGameAPITrack track);
	CSAssistGameAPITrack connectCSAssistInternal();
	void ConnectLB();
	void GetLBHost();
	void OnConnectLB(unsigned track, unsigned result, std::string serverName, unsigned serverPort, Request *, Response *);
#ifdef USE_UDP_LIBRARY
	inline bool actuallyConnected()		{ return (bool)((m_connection != nullptr) && (m_connection->GetStatus() == UdpConnection::cStatusConnected)); }
	UdpManager				*m_conManager;
	UdpConnection			*m_connection;
#else
	inline bool actuallyConnected()		{ return (bool)((m_connection != nullptr) && (m_connection->GetStatus() == TcpConnection::StatusConnected)); }
	TcpManager				*m_conManager;
	TcpConnection			*m_connection;
#endif

	short					m_port;
	short					m_oldPort;
	short					m_connectedPort;
	std::string				m_ip;							// this is supposed to be non-internationalised
	std::string				m_oldIP;						// this is supposed to be non-internationalised
	std::string				m_connectedIP;					// this is supposed to be non-internationalised
	std::list<ServerID>		m_serverList;
	std::list<ServerID>::iterator	m_curServer;
	CSAssistReceiver		*m_receiver;
	CSAssistGameAPI			*m_api;
	std::map<CSAssistGameAPITrack, Response *> m_pending;
	std::queue<timeout *> 	m_timeout;
	std::queue<Request *> 	m_outQueue;
	std::queue<Response *> 	m_error;
	CSAssistGameAPITrack	m_currTrack;

	std::map<CSAssistGameAPITrack, Response *> m_pendingInt;
	std::queue<timeout *> 	m_timeoutInt;
	std::queue<Request *> 	m_outQueueInt;
	std::queue<Response *> 	m_errorInt;
	CSAssistGameAPITrack	m_currTrackInt;

	time_t					m_reconnectTimeout;
	Plat_Unicode::String	m_identityGame;
	Plat_Unicode::String	m_identityServer;
	unsigned				m_userTimeout;
	unsigned				m_flags;
	std::multimap<unsigned, RegisteredCharacter> m_registeredCharacters;
	std::set<unsigned>		m_internalTracks;
	int						m_numberPacketsSendPerSec;
	time_t					m_gatingSecond;
	int						m_packetsLeftThisSecond;
	std::multimap<unsigned, RegisteredCharacter> m_registeredCharactersCopy;
	bool					m_useRedirectServer;
	bool					m_isConnectedToCSAServer;
	bool					m_canConnect;
	bool					m_useFailoverHosts;
	bool					m_connectFailed;
	CONNECT_STATE			m_connectState;
	unsigned				m_connectReqCount;
	unsigned				m_lastConnectReqTrack;
};


} // namespace CSAssist

#endif

