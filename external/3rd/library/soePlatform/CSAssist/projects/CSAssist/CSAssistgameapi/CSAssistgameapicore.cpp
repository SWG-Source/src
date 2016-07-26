//------------------------------------------------------------------------------
//
// CSAssistgameapicore.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// API core
//
//------------------------------------------------------------------------------
#pragma warning (disable: 4786)

#include "CSAssistgameapicore.h"
#include "CSAssistreceiver.h"
#include "CSAssistgameapi.h"
#include "response.h"
#include "request.h"
#include "Unicode/UnicodeUtils.h"

#pragma warning (disable : 4786)

namespace CSAssist
{
	using namespace Base;
	using namespace std;
	using namespace Plat_Unicode;

	//--------------------------------------
	CSAssistGameAPIcore::CSAssistGameAPIcore(CSAssistGameAPI *api, const char *host, const unsigned port, const unsigned timeout, const unsigned flags)
		: m_connection(nullptr),
		m_port(port),
		m_oldPort(port),
		m_connectedPort(0),
		m_ip(host),
		m_oldIP(host),
		m_api(api),
		m_currTrack(1),
		m_currTrackInt(1),
		m_userTimeout(timeout),
		m_flags(flags),
		m_numberPacketsSendPerSec(MAX_PACKETS_PER_SEC),
		m_gatingSecond(0),
		m_packetsLeftThisSecond(0),
		m_useRedirectServer((flags & CSASSIST_APIFLAG_NO_REDIRECT) ? false : true),
		m_canConnect(true),
		m_useFailoverHosts(false),
		m_connectFailed(false),
		m_connectState(CONNECT_INIT)
		, m_connectReqCount(0)
		, m_lastConnectReqTrack(0)
		///--------------------------------------
	{
		m_identityGame = narrowToWide("");
		m_identityServer = narrowToWide("");

		if (timeout == 0)
			m_userTimeout = NORMAL_TIMEOUT_VAL;

#ifdef USE_UDP_LIBRARY
		UdpManager::Params params;
		params.keepAliveDelay = 1000;
		params.incomingBufferSize = 32 * 1024;
		params.outgoingBufferSize = 32 * 1024;
		params.maxConnections = 1;
		params.port = 0;
		m_conManager = new UdpManager(&params);
		m_receiver = new CSAssistReceiver(this);
#else
		TcpManager::TcpParams params;
		params.keepAliveDelay = 1000;
		params.noDataTimeout = 0;
		params.incomingBufferSize = 32 * 1024;
		params.outgoingBufferSize = 32 * 1024;
		params.maxRecvMessageSize = 4096 * 1024;
		params.maxConnections = 1;
		params.port = 0;
		m_conManager = new TcpManager(params);
		m_receiver = new CSAssistReceiver(this);
		m_conManager->SetHandler(m_receiver);
#endif

		// make an initial call to start the connection process.
		// this prevents the first couple of requests from immediately timing out.
		//Update();
	}

	CSAssistGameAPIcore::CSAssistGameAPIcore(CSAssistGameAPI *api, const char *serverList, const unsigned timeout, const unsigned flags)
		: m_connection(nullptr),
		m_connectedPort(0),
		m_api(api),
		m_currTrack(1),
		m_currTrackInt(1),
		m_userTimeout(timeout),
		m_flags(flags),
		m_numberPacketsSendPerSec(MAX_PACKETS_PER_SEC),
		m_gatingSecond(0),
		m_packetsLeftThisSecond(0),
		m_useRedirectServer((flags & CSASSIST_APIFLAG_NO_REDIRECT) ? false : true),
		m_canConnect(true),
		m_useFailoverHosts(true),
		m_connectFailed(false),
		m_connectState(CONNECT_INIT)
		, m_connectReqCount(0)
		, m_lastConnectReqTrack(0)
		//--------------------------------------
	{
		///////////////////////////////////////////
		// populate server list
		int size = 0;

		if (serverList != nullptr) {
			size = ::strlen(serverList);
		}

		if (!serverList || (size <= 0))
		{
			fprintf(stderr, "CSASsistGameAPIcore()::serverList==nullptr! aborting...\n");
			abort();
		}
		char *p2 = nullptr;
		char *buf = new char[size + 1];
		char delims[] = " ,";
		char *host = new char[size];
		unsigned port(0);
		memset(buf, 0, size + 1);
		strncpy(buf, serverList, size);
		list<std::string> servers;
		p2 = strtok(buf, delims);
		while (p2)
		{
			servers.push_back(p2);
			p2 = strtok(nullptr, delims);
		}
		for (list<string>::iterator iter = servers.begin(); iter != servers.end(); iter++)
		{
			char *p = new char[iter->size() + 1];
			memset(p, 0, iter->size() + 1);
			strncpy(p, iter->c_str(), iter->size());
			ServerID sid;
			memset(host, 0, size);
			sprintf(host, "%s", strtok(p, ":"));
			int res(1);
			char *pc = strtok(nullptr, ":");
			if (pc) { port = atoi(pc); res++; }
			//if (res == 2)
			{
				sid.host = (string)host;
				sid.port = port;
				m_serverList.push_back(sid);
			}
			//fprintf(stderr, "res=%d, p=%s, host=%s, port=%d, size=%d\n", res, p, host, port, m_serverList.size());
			delete[] p;
		}
		delete[] host;
		delete[] buf;
		m_curServer = m_serverList.begin();
		GetLBHost();
		//
		/////////////////////////////////////////////

		m_identityGame = narrowToWide("");
		m_identityServer = narrowToWide("");

		if (timeout == 0)
			m_userTimeout = NORMAL_TIMEOUT_VAL;

#ifdef USE_UDP_LIBRARY
		UdpManager::Params params;
		params.keepAliveDelay = 1000;
		params.incomingBufferSize = 32 * 1024;
		params.outgoingBufferSize = 32 * 1024;
		params.maxConnections = 1;
		params.port = 0;
		m_conManager = new UdpManager(&params);
		m_receiver = new CSAssistReceiver(this);
#else
		TcpManager::TcpParams params;
		params.keepAliveDelay = 1000;
		params.noDataTimeout = 0;
		params.incomingBufferSize = 32 * 1024;
		params.outgoingBufferSize = 32 * 1024;
		params.maxRecvMessageSize = 4096 * 1024;
		params.maxConnections = 1;
		params.port = 0;
		m_conManager = new TcpManager(params);
		m_receiver = new CSAssistReceiver(this);
		m_conManager->SetHandler(m_receiver);
#endif

		// make an initial call to start the connection process.
		// this prevents the first couple of requests from immediately timing out.
		//Update();
	}

	//--------------------------------------
	CSAssistGameAPIcore::~CSAssistGameAPIcore()
		//--------------------------------------
	{
		if (m_connection)
		{
#ifdef USE_UDP_LIBRARY
			m_connection->Disconnect();
#else
			m_connection->Disconnect(false);
#endif
			m_connection->Release();
			m_connection = nullptr;
		}
		m_conManager->Release();
		delete m_receiver;

		// ----- clear pending map -----
		map<CSAssistGameAPITrack, Response *>::iterator iter;
		for (iter = m_pending.begin(); iter != m_pending.end(); iter++)
		{
			delete (*iter).second;
		}
		m_pending.clear();

		for (iter = m_pendingInt.begin(); iter != m_pendingInt.end(); iter++)
		{
			delete (*iter).second;
		}
		m_pendingInt.clear();

		// ----- clear timeout queues -----
		while (m_timeout.size() > 0)
		{
			delete m_timeout.front();
			m_timeout.pop();
		}
		while (m_timeoutInt.size() > 0)
		{
			delete m_timeoutInt.front();
			m_timeoutInt.pop();
		}

		// ----- clear error queues -----
		while (m_error.size() > 0)
		{
			delete m_error.front();
			m_error.pop();
		}
		while (m_errorInt.size() > 0)
		{
			delete m_errorInt.front();
			m_errorInt.pop();
		}

		// ----- clear request queues -----
		while (m_outQueue.size() > 0)
		{
			delete m_outQueue.front();
			m_outQueue.pop();
		}
		while (m_outQueueInt.size() > 0)
		{
			delete m_outQueueInt.front();
			m_outQueueInt.pop();
		}

		// ----- clear registered character map -----
		/*for(multimap<unsigned, RegisteredCharacter *>::iterator iter1 = m_registeredCharacters.begin(); iter1 != m_registeredCharacters.end(); iter1++)
		{
			delete (*iter1).second;
		}*/
		m_registeredCharacters.clear();
		// ----- clear registered character copy map -----
		/*for(multimap<unsigned, RegisteredCharacter *>::iterator iterC = m_registeredCharactersCopy.begin(); iterC != m_registeredCharactersCopy.end(); iterC++)
		{
			delete (*iterC).second;
		}*/
		m_registeredCharactersCopy.clear();

		m_internalTracks.clear();
	}

	/*
		Request*: Submit request packets to send queue so they will eventually be sent by udp to the CSAssist server.
		Each function should create the appropriate request message(defined in request.h), call SubmitRequest, and finally
		increment track by one, and return the track number of the submitted request to the interface level

		Any dynamically allocated variables, strings, credit card structs etc must be copied locally and handed into the request
		packet itself to avoid scoping problems
	*/

	//--------------------------------------
	void CSAssistGameAPIcore::SubmitRequest(Request *req, Response *res)
		//--------------------------------------
	{
		if (req->getType() != CSASSIST_CALL_CONNECTLB)
		{
			m_timeout.push(new timeout(res->getTrack(), time(nullptr) + m_userTimeout));
			req->setTimeout(time(nullptr) + m_userTimeout);
		}
		else
		{
			m_timeout.push(new timeout(res->getTrack(), time(nullptr) + CONNECTLB_TIMEOUT));
			req->setTimeout(time(nullptr) + CONNECTLB_TIMEOUT);
		}
		m_outQueue.push(req);
		m_pending.insert(pair<CSAssistGameAPITrack, Response *>(res->getTrack(), res));
	}

	//--------------------------------------
	void CSAssistGameAPIcore::SubmitRequestInt(Request *req, Response *res)
		//--------------------------------------
	{
		if (req->getType() != CSASSIST_CALL_CONNECTLB)
		{
			m_timeoutInt.push(new timeout(res->getTrack(), time(nullptr) + m_userTimeout));
			req->setTimeout(time(nullptr) + m_userTimeout);
		}
		else
		{
			m_timeoutInt.push(new timeout(res->getTrack(), time(nullptr) + CONNECTLB_TIMEOUT));
			req->setTimeout(time(nullptr) + CONNECTLB_TIMEOUT);
		}
		m_outQueueInt.push(req);
		m_pendingInt.insert(pair<CSAssistGameAPITrack, Response *>(res->getTrack(), res));
		//fprintf(stderr, "SubmitRequestInt() Adding request type %u\n", req->getType());
	}

	//--------------------------------------
	void CSAssistGameAPIcore::SubmitImmediateError(Response *res)
		// Parameters were passed in error from the client
		// Fire back to callback by setting a timeout of 1 second
		//--------------------------------------
	{
		res->setResult(CSASSIST_RESULT_BADPARAMS);
		m_error.push(res);
		m_pending.insert(pair<CSAssistGameAPITrack, Response *>(res->getTrack(), res));
	}

	//--------------------------------------
	bool CSAssistGameAPIcore::isInternalTrackingNumber(const unsigned track)
		//--------------------------------------
	{
		set<unsigned>::iterator iter;
		iter = m_internalTracks.find(track);
		if (iter != m_internalTracks.end())
			return true;
		return false;
	}

	/*
		Callback: Take a response packet passed from the CSAssist receiver, call the appropriate callback function in
		the interface layer of the api.
	*/

	//--------------------------------------
	void CSAssistGameAPIcore::CSAssistGameCallback(Response *response)
		//--------------------------------------
	{
		unsigned track = response->getTrack();
		CSAssistGameAPIResult result = response->getResult();
		void *userData = response->getUserData();
		//fprintf(stderr, "Callback Response(%p)\n", response);

		if (response->getType() == CSASSIST_CALL_CONNECTLB || (response->getType() == CSASSIST_CALL_CONNECT && m_receiver->m_firstConnection))
			m_pendingInt.erase(track);
		else
			m_pending.erase(track);

		switch (response->getType())
		{
			// ----- **** ADD CODE HERE **** -----
		case CSASSIST_CALL_CONNECT:
			m_connectReqCount = 0;
			// call the callback to let client know a reconnect has occurred
			if (result == CSASSIST_RESULT_SUCCESS)
			{
				m_connectState = CONNECT_BACKEND_CONNECTED_AND_AUTHED;

				//if (track != 0 && track == m_receiver->getReconnectTrack())	// trap the internal reconnect response
				if (m_receiver->m_firstConnection)
				{
					// Here the API has successfully reconnected to the CSA server
					// resubmit all cached Register requests
					reregisterCharacters();								// resubmit all cached Register requests
				}
				if (!m_receiver->m_firstConnection)
					m_api->OnConnectCSAssist(track, result, userData);			// normal application layer connect
				else
					m_api->OnConnectCSAssist(0, result, nullptr);					// internal re-connect

				m_receiver->m_firstConnection = true;
			}
			else
			{
				if (m_receiver->m_firstConnection)
					m_api->OnConnectRejectedCSAssist(0, result, nullptr);
				else
					m_api->OnConnectRejectedCSAssist(track, result, userData);

				if (m_connectState == CONNECT_BACKEND_CONNECTED)
				{
					//m_canConnect	= false;
					m_connectState = CONNECT_BACKEND_DISCONNECTED;
					if (m_connection)
					{
#ifdef USE_UDP_LIBRARY
						m_connection->Disconnect();
#else
						m_connection->Disconnect(false);
#endif
						m_connection->Release();
						m_connection = nullptr;
					}
					GetLBHost();
					m_receiver->m_firstConnection = true;
			}
				m_connectFailed = true;
		}
			break;
		case CSASSIST_CALL_DISCONNECT:
		{
			m_canConnect = false;
			m_connectFailed = false;
			m_connectState = CONNECT_BACKEND_DISCONNECTED;
			m_connectReqCount = 0;
			m_receiver->m_firstConnection = false;
			if (m_connection)
			{
#ifdef USE_UDP_LIBRARY
				m_connection->Disconnect();
#else
				m_connection->Disconnect(false);
#endif
				m_connection->Release();
				m_connection = nullptr;
			}
			m_api->OnDisconnectCSAssist(track, result, userData);
		}
		break;
		case CSASSIST_CALL_NEWTICKETACTIVITY:
			m_api->OnNewTicketActivity(track, result, userData, ((ResNewTicketActivity *)response)->getNewActivityFlag(), ((ResNewTicketActivity *)response)->getHasTickets());
			break;
		case CSASSIST_CALL_REGISTERCHARACTER:
			if (track != 0 && isInternalTrackingNumber(track))
			{
				m_internalTracks.erase(track);
				break;												// make no callback for an internal request
			}														// i.e. re-registering characters after a disconnect
			m_api->OnRegisterCharacter(track, result, userData);
			break;
		case CSASSIST_CALL_UNREGISTERCHARACTER:
			m_api->OnUnRegisterCharacter(track, result, userData);
			break;
		case CSASSIST_CALL_GETISSUEHIERARCHY:
			m_api->OnGetIssueHierarchy(track, result, userData, ((ResGetIssueHierarchy *)response)->getHierarchyBody(), ((ResGetIssueHierarchy *)response)->getModifyDate());
			break;
		case CSASSIST_CALL_CREATETICKET:
			m_api->OnCreateTicket(track, result, userData, ((ResCreateTicket *)response)->getTicketID());
			break;
		case CSASSIST_CALL_APPENDCOMMENT:
			m_api->OnAppendTicketComment(track, result, userData, ((ResAppendComment *)response)->getTicketID());
			break;
		case CSASSIST_CALL_GETTICKETBYID:
			m_api->OnGetTicketByID(track, result, userData, &((ResGetTicketByID *)response)->getTicketBody());
			break;
		case CSASSIST_CALL_GETTICKETCOMMENTS:
			m_api->OnGetTicketComments(track, result, userData, ((ResGetTicketComments *)response)->getCommentArray(), ((ResGetTicketComments *)response)->getNumberRead());
			break;
		case CSASSIST_CALL_GETTICKET:
			m_api->OnGetTicketByCharacter(track, result, userData, ((ResGetTicketByCharacter *)response)->getNumberReturned(), ((ResGetTicketByCharacter *)response)->getTotalNumber(), ((ResGetTicketByCharacter *)response)->getTicketArray());
			break;
		case CSASSIST_SERVER_TICKETCHANGE:
			m_api->OnTicketChange(((ResTicketChange *)response)->getTicketID(), ((ResTicketChange *)response)->getUID(), ((ResTicketChange *)response)->getCharacter());
			break;
		case CSASSIST_CALL_MARKREAD:
			m_api->OnMarkTicketRead(track, result, userData);
			break;
		case CSASSIST_CALL_CANCELTICKET:
			m_api->OnCancelTicket(track, result, userData, ((ResCancelTicket *)response)->getTicketID());
			break;
		case CSASSIST_CALL_COMMENTCOUNT:
			m_api->OnGetTicketCommentsCount(track, result, userData, ((ResCommentCount *)response)->getTicketID(), ((ResCommentCount *)response)->getCount());
			break;
		case CSASSIST_SERVER_REQUESTLOCATION:
			m_api->OnRequestGameLocation(((ResRequestGameLocation *)response)->getSourceTrack(), ((ResRequestGameLocation *)response)->getUID(), ((ResRequestGameLocation *)response)->getCharacter(), ((ResRequestGameLocation *)response)->getCSRUID());
			break;
		case CSASSIST_CALL_REPLYLOCATION:
			break;							// no callback for this reply
		case CSASSIST_CALL_GETDOCUMENTLIST:
			m_api->OnGetDocumentList(track, result, userData, ((ResGetDocumentList *)response)->getDocumentList(), ((ResGetDocumentList *)response)->getNumberRead());
			break;
		case CSASSIST_CALL_GETDOCUMENT:
			m_api->OnGetDocument(track, result, userData, ((ResGetDocument *)response)->getDocumentBody());
			break;
		case CSASSIST_CALL_GETTICKETXMLBLOCK:
			m_api->OnGetTicketXMLBlock(track, result, userData, ((ResGetTicketXMLBlock *)response)->getXMLBody());
			break;
		case CSASSIST_CALL_GETKBARTICLE:
			m_api->OnGetKBArticle(track, result, userData, ((ResGetKBArticle *)response)->getArticleBody(), ((ResGetKBArticle *)response)->getArticleTitle());
			break;
		case CSASSIST_CALL_SEARCHKB:
			m_api->OnSearchKB(track, result, userData, ((ResSearchKB *)response)->getArticleList(), ((ResSearchKB *)response)->getNumberRead());
			break;
		case CSASSIST_SERVER_HIERARCHYCHANGE:
			m_api->OnIssueHierarchyChanged(((ResHierarchyChange *)response)->getVersion(), ((ResHierarchyChange *)response)->getLanguage(), ((ResHierarchyChange *)response)->getChangeType());
			break;
		case CSASSIST_CALL_CONNECTLB:
		{
			ResConnectLB *res = static_cast<ResConnectLB*>(response);
			this->OnConnectLB(response->getTrack(), response->getResult(),
				res->getServerName(), res->getServerPort(), res->getRequest(), res->getResponse());
		}
		break;
		//		default:
		//			fprintf(stderr, "Unknown response type: %d\n\r", response->getType());
	}
		//fprintf(stderr,"\nGameCallback() Track: %d, Type: %d exit\n", response->getTrack(), response->getType());
		delete response;
}

	//--------------------------------------
	void CSAssistGameAPIcore::localRegisterCharacter(const unsigned uid, const Plat_Unicode::String character, const unsigned avaconID)
		//--------------------------------------
	{
		//RegisteredCharacter *obj = new RegisteredCharacter(uid, character, avaconID);
		RegisteredCharacter obj(uid, character, avaconID);
		//	fprintf("\nLocalRegister: %d:%s", uid, wideToNarrow(character).c_str());		//debug
		m_registeredCharacters.insert(pair<unsigned, RegisteredCharacter>(uid, obj));
	}

	//--------------------------------------
	void CSAssistGameAPIcore::localUnRegisterCharacter(const unsigned uid, const Plat_Unicode::String character)
		//--------------------------------------
	{
		multimap<unsigned, RegisteredCharacter>::iterator iter;
		//fprintf(stderr, "\nTrying to UnRegister: %d:%s\n", uid, wideToNarrow(character).c_str());	//debug

		// Remove character from the list of characters
		iter = m_registeredCharacters.find(uid);
		if (iter != m_registeredCharacters.end())
		{
			do
			{
				RegisteredCharacter &obj = (*iter).second;
				if (character == obj.getCharacter())
				{
					//fprintf(stderr, "\nErasing: %d:%s\n", obj.getUID(), wideToNarrow(obj.getCharacter()).c_str());	//debug
					m_registeredCharacters.erase(iter);	// registered by character string. Erase just this element
					break;
				}
				iter++;
			} while (iter != m_registeredCharacters.upper_bound(uid));
		}

		// Now remove from the cached copy of the list used for failover
		iter = m_registeredCharactersCopy.find(uid);
		if (iter != m_registeredCharactersCopy.end())
		{
			do
			{
				RegisteredCharacter &obj = (*iter).second;
				if (character == obj.getCharacter())
				{
					//fprintf(stderr, "\nErasing: %d:%s\n", obj.getUID(), wideToNarrow(obj.getCharacter()).c_str());	//debug
					m_registeredCharactersCopy.erase(iter);	// registered by character string. Erase just this element
					break;
				}
				iter++;
			} while (iter != m_registeredCharactersCopy.upper_bound(uid));
		}
	}

	//--------------------------------------
	void CSAssistGameAPIcore::reregisterCharacters()
		// copy the registered character map
		//--------------------------------------
	{
		// clear existing map first in case it has anything in it!
		/*for(multimap<unsigned, RegisteredCharacter *>::iterator iter1 = m_registeredCharactersCopy.begin(); iter1 != m_registeredCharactersCopy.end(); iter1++)
		{
			delete (*iter1).second;
		}
		*/
		m_registeredCharactersCopy.clear();

		m_registeredCharactersCopy = m_registeredCharacters;
	}

	//--------------------------------------
	void CSAssistGameAPIcore::batchRegisterCharacters(const unsigned numToSend)
		//--------------------------------------
	{
		if (m_registeredCharactersCopy.size() == 0)
			return;

		//	fprintf("\nReregistering %d chars", numToSend);		//debug
		multimap<unsigned, RegisteredCharacter>::iterator iter = m_registeredCharactersCopy.begin();
		multimap<unsigned, RegisteredCharacter>::iterator current;
		for (unsigned i = 0; i < numToSend; i++)
		{
			if (iter == m_registeredCharactersCopy.end())
				return;
			RegisteredCharacter &obj = (*iter).second;
			ResRegisterCharacter *Res = new ResRegisterCharacter(m_currTrack, 0);
			RRegisterCharacter *R = new RRegisterCharacter(m_currTrack, obj.getUID(), obj.getCharacter(), obj.getAvaconID());
			//fprintf(stderr, "\nRE-Register: %d:%s", obj->getUID(), wideToNarrow(obj->getCharacter()).c_str());		//debug
			SubmitRequest(R, Res);
			m_internalTracks.insert(m_currTrack);
			m_currTrack++;
			current = iter++;
			m_registeredCharactersCopy.erase(current);
		}
	}

	//--------------------------------------
	Response * CSAssistGameAPIcore::getPending(CSAssistGameAPITrack track)
		//--------------------------------------
	{
		map<CSAssistGameAPITrack, Response *>::iterator iter;
		iter = m_pending.find(track);
		if (iter == m_pending.end())
			return nullptr;
		return (*iter).second;
	}

	//--------------------------------------
	Response * CSAssistGameAPIcore::getPendingInt(CSAssistGameAPITrack track)
		//--------------------------------------
	{
		map<CSAssistGameAPITrack, Response *>::iterator iter;
		iter = m_pendingInt.find(track);
		if (iter == m_pendingInt.end())
			return nullptr;
		return (*iter).second;
	}

	/*
		Update: Attempt to reconnect if the connection was broken, discharge timed out packets, process send queue, and perform
		callbacks on result messages from CSAssist Server
	*/

	//--------------------------------------
	void CSAssistGameAPIcore::Update()
		//--------------------------------------
	{
		timeout *t;

		// ----- update packet gating -----
		time_t now = time(0);
		if (now != m_gatingSecond)
		{
			m_gatingSecond = now;
			m_packetsLeftThisSecond = m_numberPacketsSendPerSec;
			// every second, put another 'n' register resends into the buffer
			// we are interleaving them with other requests coming in
			batchRegisterCharacters(m_packetsLeftThisSecond);
		}

		// ----- process immediate errors. User called with invalid params, make callback NOW -----
		while (m_error.size() > 0)
		{
			Response *Res = m_error.front();
			//fprintf(stderr, "processing errors: response(%p)\n", Res);
			CSAssistGameCallback(Res);
			m_error.pop();
			//		delete Res;			// deleted from within callback
		}
		while (m_errorInt.size() > 0)
		{
			Response *Res = m_errorInt.front();
			//fprintf(stderr, "processing internal errors: response(%p)\n", Res);
			CSAssistGameCallback(Res);
			m_errorInt.pop();
			//		delete Res;			// deleted from within callback
		}

		// ----- Process timeout queue, send timeout messages when appropriate -----
		while ((m_timeout.size() > 0) && ((t = m_timeout.front())->time <= time(nullptr)))
		{
			Response *Res = getPending(t->track);
			//fprintf(stderr, "processing timeouts: track(%u) response(%p)\n", t->track, Res);
			if (Res != nullptr)
			{
				CSAssistGameCallback(Res);
			}
			m_timeout.pop();
			delete t;
		}
		while ((m_timeoutInt.size() > 0) && ((t = m_timeoutInt.front())->time <= time(nullptr)))
		{
			Response *Res = getPendingInt(t->track);
			//fprintf(stderr, "processing internal timeouts: track(%u) response(%p)\n", t->track, Res);
			if (Res != nullptr)
			{
				CSAssistGameCallback(Res);
			}
			m_timeoutInt.pop();
			delete t;
		}

		// ----- process timeouts for requests -----
		while ((m_outQueue.size() > 0) && (m_outQueue.front()->getTimeout() <= (unsigned)time(nullptr)))
		{
			Request *R = m_outQueue.front();
			//fprintf(stderr, "processing request timeouts: track(%u) request(%u)\n", R->getTrack(), R->getType());
			m_outQueue.pop();
			delete R;
		}
		while ((m_outQueueInt.size() > 0) && (m_outQueueInt.front()->getTimeout() <= (unsigned)time(nullptr)))
		{
			Request *R = m_outQueueInt.front();
			//fprintf(stderr, "processing internal request timeouts: track(%u) request(%u)\n", R->getTrack(), R->getType());
			m_outQueueInt.pop();
			delete R;
		}

		if (!m_connection && m_canConnect)
		{
			// API does not have a connection, begin connection handshake process
			//fprintf(stderr, "Going to connect to %s:%d\n", m_ip.c_str(), m_port);
			m_reconnectTimeout = time(nullptr) + 5;
			m_connection = m_conManager->EstablishConnection(m_ip.c_str(), m_port);

			// set connected host/port before changing with GetLBHost.
			m_connectedIP = m_ip.c_str();
			m_connectedPort = m_port;

			if (m_connection != nullptr)
				m_connection->SetHandler(m_receiver);
			else
			{
				m_api->OnConnectRejectedCSAssist(0, CSASSIST_RESULT_FAIL, nullptr);
				// Connection failed. Try getting a new host.
				GetLBHost();
			}

			switch (m_connectState)
			{
			case CONNECT_INIT:
				if (m_useRedirectServer) m_connectState = CONNECT_REDIRECT_NEGOTIATING;
				else m_connectState = CONNECT_BACKEND_NEGOTIATING;
				break;
			case CONNECT_REDIRECT_INIT:
			case CONNECT_REDIRECT_NEGOTIATING:
			case CONNECT_REDIRECT_DISCONNECTED: m_connectState = CONNECT_REDIRECT_NEGOTIATING; break;
			case CONNECT_BACKEND_INIT:			m_connectState = CONNECT_BACKEND_NEGOTIATING; break;
			case CONNECT_BACKEND_NEGOTIATING:
			case CONNECT_BACKEND_DISCONNECTED:
				if (m_useRedirectServer) m_connectState = CONNECT_REDIRECT_NEGOTIATING;
				else m_connectState = CONNECT_BACKEND_NEGOTIATING;
				break;
			default:
				fprintf(stderr, "CSAssistGameAPI::Update(1): error, m_connectState = %u\n", m_connectState);
			}
			//fprintf(stderr, "debug: Update(1): Now m_connectState=%u\n", m_connectState);

			// ----- lost connection, so timeout all pending requests right now -----
			if ((m_flags & CSASSIST_APIFLAG_ASSUME_RECONNECT) == 0)
			{
				unsigned qsize = m_timeout.size();
				for (unsigned i(0); i < qsize; i++)
				{
					t = m_timeout.front();
					Response *Res = getPending(t->track);
					if (Res && (Res->getType() == CSASSIST_CALL_CONNECT || Res->getType() == CSASSIST_CALL_CONNECTLB))
					{
						m_timeout.pop();
						m_timeout.push(t);
					}
					else
					{
						//fprintf(stderr,"Update(1): timing out response(%p) for track(%u)\n", Res, t->track);
						if (Res != nullptr)
						{
							CSAssistGameCallback(Res);
						}
						m_timeout.pop();
						delete t;
					}
				}
			}
		}
		else
		{
			// API has a connection in some state
			// Check for disconnect or a connection that has been in the negotiating
			// state for too long and close it
			//fprintf(stderr, "Update(2) enter\n");
#ifdef USE_UDP_LIBRARY
			if ((m_connection->GetStatus() == UdpConnection::cStatusDisconnected) ||
				(m_connection->GetStatus() == UdpConnection::cStatusNegotiating && m_reconnectTimeout < time(nullptr)))
			{
				m_connection->Disconnect();
#else
			if ((m_connection->GetStatus() == TcpConnection::StatusDisconnected) ||
				(m_connection->GetStatus() == TcpConnection::StatusNegotiating && m_reconnectTimeout < time(nullptr)))
			{
				m_connection->Disconnect(false);
#endif

				m_connection->Release();
				m_connection = nullptr;
				//fprintf(stderr, "Update(2): Disconnected!! m_connectState=%u\n", m_connectState);

				switch (m_connectState)
				{
				case CONNECT_INIT:
					if (m_useRedirectServer) m_connectState = CONNECT_REDIRECT_INIT;
					else m_connectState = CONNECT_BACKEND_INIT;
					break;
				case CONNECT_REDIRECT_INIT:
				case CONNECT_REDIRECT_NEGOTIATING:
					m_api->OnConnectRejectedCSAssist(0, CSASSIST_RESULT_TIMEOUT, 0); // notify app we could not connect to this host
					GetLBHost();
				case CONNECT_REDIRECT_DISCONNECTED: m_connectState = CONNECT_REDIRECT_INIT;	break;
				case CONNECT_BACKEND_INIT:
				case CONNECT_BACKEND_NEGOTIATING:
					m_api->OnConnectRejectedCSAssist(0, CSASSIST_RESULT_TIMEOUT, 0); // notify app we could not connect to this host
					GetLBHost();
				case CONNECT_BACKEND_DISCONNECTED:
					if (m_useRedirectServer) m_connectState = CONNECT_REDIRECT_INIT;
					else
					{
						unsigned qsize = m_timeoutInt.size();
						for (unsigned i(0); i < qsize; i++)
						{
							timeout *t = m_timeoutInt.front();
							//if (t->
							Response *Res = getPendingInt(t->track);
							//fprintf(stderr, "Update(2) timing out connect: track(%u) response(%p)\n", t->track, Res);
							if (Res != nullptr)
							{
								Res->setResult(CSASSIST_RESULT_FAIL);
								CSAssistGameCallback(Res);
							}
							m_timeoutInt.pop();
							if (Res && Res->getType() == CSASSIST_CALL_CONNECT)
								delete t;
							else
								m_timeoutInt.push(t);
						}
						m_connectState = CONNECT_BACKEND_INIT;
					}
					break;
				default:
					fprintf(stderr, "CSAssistGameAPI::Update(2): error, m_connectState = %u\n", m_connectState);
				}
				//fprintf(stderr, "debug: Update(2): Now m_connectState=%u\n", m_connectState);
			}
			else if (actuallyConnected() && ((m_outQueue.size() > 0) || (m_outQueueInt.size() > 0)))
			{
				// Have a live connection and pending requests
				//fprintf(stderr, "Update(3) enter\n");
				if (m_connectState == CONNECT_REDIRECT_CONNECTED ||
					(m_connectState == CONNECT_BACKEND_CONNECTED && m_receiver->m_firstConnection))
				{
					unsigned qsize = m_outQueueInt.size();
					for (unsigned loop = 0; loop < qsize; loop++)
					{
						//fprintf("\nPackets Left this Sec = %d", m_packetsLeftThisSecond);	// debug
						//if (m_packetsLeftThisSecond <= 0)
						//	break;									// cannot send any more this second!
						//else
						//	m_packetsLeftThisSecond--;
						ByteStream msg;
						Request *R = m_outQueueInt.front();
						//if (m_useRedirectServer)
						//{
						if (m_connectState == CONNECT_REDIRECT_CONNECTED && R->getType() == CSASSIST_CALL_CONNECTLB)
						{
							{
								//fprintf(stderr, "Sending CALL_CONNECTLB request\n");
								R->pack(msg);
#ifdef USE_UDP_LIBRARY
								m_connection->Send(cUdpChannelReliable1, msg.getBuffer(), msg.getSize());
#else
								m_connection->Send((const char *)msg.getBuffer(), msg.getSize());
#endif
							}
							m_outQueueInt.pop();
							delete R;
						}
						else if (m_connectState == CONNECT_BACKEND_CONNECTED && R->getType() == CSASSIST_CALL_CONNECT)
						{
							{
								//fprintf(stderr, "Sending internal CALL_CONNECT request\n");
								R->pack(msg);
#ifdef USE_UDP_LIBRARY
								m_connection->Send(cUdpChannelReliable1, msg.getBuffer(), msg.getSize());
#else
								m_connection->Send((const char *)msg.getBuffer(), msg.getSize());
#endif
							}
							//else
							//{
							//	fprintf(stderr, "NOT !!!!! Sending CALL_CONNECT request\n");
							//}
							m_outQueueInt.pop();
							delete R;
						}
						else
						{
							//m_outQueue.pop();
							//m_outQueue.push(R);
						}
					}
				}
				else
				{
					unsigned qsize = m_outQueue.size();
					for (unsigned loop = 0; loop < qsize; loop++)
					{
						//fprintf("\nPackets Left this Sec = %d", m_packetsLeftThisSecond);	// debug
						if (m_packetsLeftThisSecond <= 0)
							break;									// cannot send any more this second!
						else
							m_packetsLeftThisSecond--;

						ByteStream msg;
						Request *R = m_outQueue.front();

						if ((m_connectState == CONNECT_BACKEND_CONNECTED && R->getType() == CSASSIST_CALL_CONNECT)
							|| m_connectState == CONNECT_BACKEND_CONNECTED_AND_AUTHED)
						{
							//if (R->getType() == CSASSIST_CALL_CONNECT)
							//	fprintf(stderr, "Sending CALL_CONNECT request\n");

							R->pack(msg);
#ifdef USE_UDP_LIBRARY
							m_connection->Send(cUdpChannelReliable1, msg.getBuffer(), msg.getSize());
#else
							m_connection->Send((const char *)msg.getBuffer(), msg.getSize());
#endif
							m_outQueue.pop();
							delete R;
						}
						else
						{
							m_outQueue.pop();
							m_outQueue.push(R);
						}
					}
				}
			}
			}

		m_conManager->GiveTime();
				}

	//-------------------------------------------------------------
	Response *CSAssistGameAPIcore::createServerResponse(short msgtype)
		//-------------------------------------------------------------
	{
		Response *res = nullptr;

		switch (msgtype)
		{
		case CSASSIST_SERVER_TICKETCHANGE:
			res = new ResTicketChange(0);
			break;
		case CSASSIST_SERVER_REQUESTLOCATION:
			res = new ResRequestGameLocation(0);
			break;
		case CSASSIST_SERVER_HIERARCHYCHANGE:
			res = new ResHierarchyChange(0);
			break;
			//		default:
			//			fprintf(stderr, "Unknown server response type: %d\n\r", msgtype);
			//			res = 0;
		}
		return res;
	}

	const char* CSAssistGameAPIcore::GetConnectedHost()
	{
		return m_connectedIP.c_str();
		/*	std::string host;
		#ifdef USE_UDP_LIBRARY
		#else
			char buf[512] = {0};
			IPAddress ip = m_connection->GetDestinationIp();
			ip.GetAddress(buf);
			host = buf;
		#endif

			return host.c_str();
		*/
	}

	int	CSAssistGameAPIcore::GetConnectedPort()
	{
		return m_connectedPort;
		/*	short port(0);
		#ifdef USE_UDP_LIBRARY
		#else
			port = m_connection->GetDestinationPort();
		#endif
			return port;
		*/
	}

	//----------------------------------------------------------
	void CSAssistGameAPIcore::GetLBHost()
		//----------------------------------------------------------
	{
		if (!m_useFailoverHosts)
		{
			m_ip = m_oldIP;
			m_port = m_oldPort;
		}
		else
		{
			if (m_serverList.size() < 1) fprintf(stderr, "No hosts given for failover mode\n");
			if (m_curServer == m_serverList.end())
				m_curServer = m_serverList.begin();
			m_ip = (*m_curServer).host;
			m_port = (*m_curServer).port;
			//fprintf(stderr, "GetLBHost(): host:%s, port:%d\n", m_ip.c_str(), m_port);
			m_curServer++;
		}
	}

	//---------------------------------------------------------------------
	void CSAssistGameAPIcore::OnConnectLB(unsigned track, unsigned result, std::string serverName, unsigned serverPort, Request *connectRequest, Response *connectResponse)
		//
		// We have a response for a request to connect to CSAssist through a SW connection load balancer.
		// Now we try to really connect to this host for CSAssist.
		//---------------------------------------------------------------------
	{
		//fprintf(stderr, "OnConnectLB() begin\n");
		if (result == CSASSIST_RESULT_SUCCESS)
		{
			//fprintf(stderr, "OnConnectLB(): Got serverName(%s), port(%u)\n", serverName.c_str(), serverPort);
			if (!serverName.empty() && serverPort != 0)
			{
				m_oldIP = m_ip;
				m_oldPort = m_port;
				m_ip = serverName;
				m_port = serverPort;
				m_connectState = CONNECT_BACKEND_INIT;
			}
			else
			{
				m_connectState = CONNECT_REDIRECT_INIT;
				m_connectReqCount = 0;
			}
			}
		else
		{
			//fprintf(stderr,"Error in OnConnectLB: track(%u),result(%u),serverName(%s), port(%u)\n", track,result,serverName.c_str(), serverPort);
			m_connectState = CONNECT_REDIRECT_INIT;
			m_connectReqCount = 0;
		}
		if (m_connection)
		{
			m_connection->SetHandler(0);
#ifdef USE_UDP_LIBRARY
			m_connection->Disconnect();
#else
			m_connection->Disconnect(false);
#endif
			m_connection->Release();
			m_connection = 0;
		}
		if (result == CSASSIST_RESULT_TIMEOUT)
		{
			GetLBHost();
			if (connectRequest && connectResponse)
				SubmitRequest(connectRequest, connectResponse);
			else
				m_api->OnConnectRejectedCSAssist(0, CSASSIST_RESULT_FAIL, 0);
		}
		else
		{
			if (connectRequest && connectResponse)
				SubmitRequest(connectRequest, connectResponse);
		}
		}

	//-------------------------------------------------------------
	//
	//	Implement internal API request calls here
	//	These have no callback
	//
	//-------------------------------------------------------------

	//---------------------------
	void CSAssistGameAPIcore::setMaxPacketsPerSecond(const unsigned n)
		//---------------------------
	{
		m_numberPacketsSendPerSec = n;
		m_gatingSecond = 0;					// force reload of value in Update()
	}

	//-------------------------------------------------------------
	//
	//	Implement API request calls here **** ADD CODE HERE ****
	//
	//-------------------------------------------------------------

	void CSAssistGameAPIcore::ConnectLB()
	{
		if (m_useRedirectServer && (m_connectState == CONNECT_REDIRECT_CONNECTED))
		{
			//fprintf(stderr,"Initiating internal redirect connection\n");
			// Attempt to connect to the RedirectServer (Load Balancing) to obtain
			// address of an available CSAssist Server.
			std::string connectString = wideToNarrow(m_identityGame) + ":" + wideToNarrow(m_identityServer);
			Request * req = new RConnectLB(m_currTrackInt, connectString);
			Response *res = new ResConnectLB(m_currTrackInt, 0, 0);
			this->SubmitRequestInt(req, res);
			m_currTrackInt++;
		}
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::connectCSAssist(const void *userData, const CSAssistUnicodeChar *GameName, const CSAssistUnicodeChar *ServerName)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String game = GameName;
		Plat_Unicode::String server = ServerName;
		if (m_connectReqCount > 0)
		{
			//fprintf(stderr, "connectCSAssist() m_connectReqCount=%d\n",m_connectReqCount);
			return 0;
		}
		ResConnectCSAssist *Res = new ResConnectCSAssist(m_currTrack, userData);
		if (game.size() == 0 || server.size() == 0 ||
			(game.size() > CSASSIST_TICKET_LENGTH_GAME) || (server.size() > CSASSIST_TICKET_LENGTH_SERVER))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			m_canConnect = true;
			m_identityGame = GameName;
			m_identityServer = ServerName;
			RConnectCSAssist *R = new RConnectCSAssist(m_currTrack, game, server);
			SubmitRequest(R, Res);
			//fprintf(stderr, "debug: connectCSAssist(): Now m_connectState=%u\n", m_connectState);
			m_connectReqCount++;
		}
		m_lastConnectReqTrack = m_currTrack;
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::connectCSAssistInternal()
		//-------------------------------------------------------------
	{
		ResConnectCSAssist *Res = new ResConnectCSAssist(m_currTrackInt, 0);
		if (m_identityGame.size() == 0 || m_identityServer.size() == 0 ||
			(m_identityGame.size() > CSASSIST_TICKET_LENGTH_GAME) || (m_identityServer.size() > CSASSIST_TICKET_LENGTH_SERVER))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			m_canConnect = true;
			RConnectCSAssist *R = new RConnectCSAssist(m_currTrackInt, m_identityGame, m_identityServer);
			//R->setTimeout(5);
			SubmitRequestInt(R, Res);
			//fprintf(stderr, "debug: connectCSAssistInternal(): Now m_connectState=%u\n", m_connectState);
			m_connectReqCount++;
		}
		m_lastConnectReqTrack = m_currTrackInt;
		return(m_currTrackInt++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::disconnectCSAssist(const void *userData)
		//-------------------------------------------------------------
	{
		// reset failover list
		if (m_useFailoverHosts)
		{
			m_curServer = m_serverList.end();
		}
		GetLBHost();

		m_canConnect = false;
		//m_connectState = CONNECT_DISCONNECTED;
		m_receiver->m_firstConnection = false;
		RDisconnectCSAssist *R = new RDisconnectCSAssist(m_currTrack);
		ResDisconnectCSAssist *Res = new ResDisconnectCSAssist(m_currTrack, userData);
		SubmitRequest(R, Res);
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestNewTicketActivity(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry;
		if (character != 0)
			charry = character;
		else
			charry = narrowToWide("");
		ResNewTicketActivity *Res = new ResNewTicketActivity(m_currTrack, userData);
		if (charry.size() > CSASSIST_TICKET_LENGTH_CHARACTER || uid == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RNewTicketActivity *R = new RNewTicketActivity(m_currTrack, uid, charry);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned avaconID)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry;
		if (character != 0)
			charry = character;
		else
			charry = narrowToWide("");
		ResRegisterCharacter *Res = new ResRegisterCharacter(m_currTrack, userData);
		if ((charry.size() > CSASSIST_TICKET_LENGTH_CHARACTER) || uid == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			localRegisterCharacter(uid, charry, avaconID);
			RRegisterCharacter *R = new RRegisterCharacter(m_currTrack, uid, charry, avaconID);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestUnRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry;
		if (character != 0)
			charry = character;
		else
			charry = narrowToWide("");
		ResUnRegisterCharacter *Res = new ResUnRegisterCharacter(m_currTrack, userData);
		if ((charry.size() > CSASSIST_TICKET_LENGTH_CHARACTER) || uid == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			localUnRegisterCharacter(uid, charry);
			RUnRegisterCharacter *R = new RUnRegisterCharacter(m_currTrack, uid, charry);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetIssueHierarchy(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String lang = language;
		Plat_Unicode::String ver = version;
		ResGetIssueHierarchy *Res = new ResGetIssueHierarchy(m_currTrack, userData);
		if (lang.size() == 0 || (lang.size() > CSASSIST_TICKET_LENGTH_LANGUAGE) ||
			ver.size() == 0 || (ver.size() > CSASSIST_TICKET_LENGTH_SERVER))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetIssueHierarchy *R = new RGetIssueHierarchy(m_currTrack, ver, lang);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestCreateTicket(const void *userData, const CSAssistGameAPITicket *ticketBody, const CSAssistUnicodeChar *XMLBody, const CSAssistGameAPIUID uid)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String xml;
		if (XMLBody == 0)
			xml = narrowToWide("");
		else
			xml = XMLBody;
		ResCreateTicket *Res = new ResCreateTicket(m_currTrack, userData);
		if (uid == 0 || ticketBody == 0 || strlen(ticketBody->game) == 0 || strlen(ticketBody->server) == 0
			|| strlen(ticketBody->character) == 0 || strlen(ticketBody->details) == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RCreateTicket *R = new RCreateTicket(m_currTrack, ticketBody, xml, uid);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestAppendTicketComment(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistUnicodeChar *comment)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry = character;
		Plat_Unicode::String com = comment;
		ResAppendComment *Res = new ResAppendComment(m_currTrack, userData);
		if (com.size() == 0 || ticket == 0 || uid == 0
			|| (charry.size() > CSASSIST_TICKET_LENGTH_NAME)
			|| (com.size() > CSASSIST_TICKET_LENGTH_COMMENT))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RAppendComment *R = new RAppendComment(m_currTrack, ticket, uid, charry, com);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetTicketByID(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned FlagAsRead)
		//-------------------------------------------------------------
	{
		RGetTicketByID *R = new RGetTicketByID(m_currTrack, ticket, FlagAsRead);
		ResGetTicketByID *Res = new ResGetTicketByID(m_currTrack, userData);
		SubmitRequest(R, Res);
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetTicketComments(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned Start, const unsigned Count, const CSAssistGameAPIOffset offset)
		//-------------------------------------------------------------
	{
		ResGetTicketComments *Res = new ResGetTicketComments(m_currTrack, userData);
		if (Count == 0 || ticket == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetTicketComments *R = new RGetTicketComments(m_currTrack, ticket, Start, Count, offset);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetTicketByCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned Start, const unsigned Count, const unsigned FlagAsRead)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry;
		if (character == 0)
			charry = narrowToWide("");
		else
			charry = character;
		ResGetTicketByCharacter *Res = new ResGetTicketByCharacter(m_currTrack, userData);
		if (Count == 0 || uid == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetTicketByCharacter *R = new RGetTicketByCharacter(m_currTrack, uid, charry, Start, Count, FlagAsRead);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestMarkTicketRead(const void *userData, const CSAssistGameAPITicketID ticket)
		//-------------------------------------------------------------
	{
		ResMarkTicketRead *Res = new ResMarkTicketRead(m_currTrack, userData);
		if (ticket == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RMarkTicketRead *R = new RMarkTicketRead(m_currTrack, ticket);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestCancelTicket(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *comment)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String com;
		if (comment == 0)
			com = narrowToWide("");
		else
			com = comment;
		ResCancelTicket *Res = new ResCancelTicket(m_currTrack, userData);
		if (ticket == 0 || uid == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RCancelTicket *R = new RCancelTicket(m_currTrack, ticket, uid, com);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetTicketCommentsCount(const void *userData, const CSAssistGameAPITicketID ticket)
		//-------------------------------------------------------------
	{
		ResCommentCount *Res = new ResCommentCount(m_currTrack, userData);
		if (ticket == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RCommentCount *R = new RCommentCount(m_currTrack, ticket);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::replyGameLocation(const void *userData, const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID csruid, const CSAssistUnicodeChar *locationString)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String charry = character;
		Plat_Unicode::String loc = locationString;
		ResReplyGameLocation *Res = new ResReplyGameLocation(m_currTrack, userData);
		if (sourceTrack == 0 || uid == 0 || csruid == 0
			|| (charry.size() > CSASSIST_TICKET_LENGTH_CHARACTER)
			|| loc.size() == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RReplyGameLocation *R = new RReplyGameLocation(m_currTrack, sourceTrack, uid, charry, csruid, loc);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetDocumentList(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String lang = language;
		Plat_Unicode::String ver = version;
		ResGetDocumentList *Res = new ResGetDocumentList(m_currTrack, userData);
		if (lang.size() == 0 || lang.size() > CSASSIST_TICKET_LENGTH_LANGUAGE || ver.size() > CSASSIST_TICKET_LENGTH_SERVER)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetDocumentList *R = new RGetDocumentList(m_currTrack, ver, lang);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetDocument(const void *userData, const unsigned id)
		//-------------------------------------------------------------
	{
		ResGetDocument *Res = new ResGetDocument(m_currTrack, userData);
		if (id == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetDocument *R = new RGetDocument(m_currTrack, id);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetTicketXMLBlock(const void *userData, const CSAssistGameAPITicketID id)
		//-------------------------------------------------------------
	{
		ResGetTicketXMLBlock *Res = new ResGetTicketXMLBlock(m_currTrack, userData);
		if (id == 0)
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetTicketXMLBlock *R = new RGetTicketXMLBlock(m_currTrack, id);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestGetKBArticle(const void *userData, const CSAssistUnicodeChar *id, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String lang = language;
		Plat_Unicode::String idString = id;
		ResGetKBArticle *Res = new ResGetKBArticle(m_currTrack, userData);
		if (lang.size() == 0 || (lang.size() > CSASSIST_TICKET_LENGTH_LANGUAGE) ||
			idString.size() == 0 || (idString.size() > CSASSIST_KBSEARCH_LENGTH_ID))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RGetKBArticle *R = new RGetKBArticle(m_currTrack, idString, lang, uid);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//-------------------------------------------------------------
	CSAssistGameAPITrack CSAssistGameAPIcore::requestSearchKB(const void *userData, const CSAssistUnicodeChar *searchstring, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid)
		//-------------------------------------------------------------
	{
		Plat_Unicode::String lang = language;
		Plat_Unicode::String search = searchstring;
		ResSearchKB *Res = new ResSearchKB(m_currTrack, userData);
		if (lang.size() == 0 || (lang.size() > CSASSIST_TICKET_LENGTH_LANGUAGE) ||
			search.size() == 0 || (search.size() > CSASSIST_KBSEARCH_LENGTH_TITLE))
		{
			SubmitImmediateError(Res);
		}
		else
		{
			RSearchKB *R = new RSearchKB(m_currTrack, search, lang, uid);
			SubmitRequest(R, Res);
		}
		return(m_currTrack++);
	}

	//--------------------------------------
	RegisteredCharacter::RegisteredCharacter(const unsigned uid_in, const Plat_Unicode::String name, const unsigned avacon)
		: uid(uid_in), character(name), avaconID(avacon)
		//--------------------------------------
	{
	}

	RegisteredCharacter::~RegisteredCharacter()
	{
	}
	} // namespace CSAssist