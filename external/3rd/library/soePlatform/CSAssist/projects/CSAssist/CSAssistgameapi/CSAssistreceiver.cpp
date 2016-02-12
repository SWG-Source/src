//------------------------------------------------------------------------------
//
// CSAssistreceiver.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// UDPLibrary wrapper and handler
//
//------------------------------------------------------------------------------
#pragma warning (disable: 4786)

#include "CSAssistgameapicore.h"
#include "CSAssistreceiver.h"
#include "response.h"

namespace CSAssist
{

using namespace Base;

extern CSAssistUnicodeChar *get_c_str(Plat_Unicode::String s);
extern CSAssistUnicodeChar *copy_c_str(Plat_Unicode::String s, CSAssistUnicodeChar *dest);



CSAssistReceiver::CSAssistReceiver(CSAssistGameAPIcore *core) 
: m_api(core), m_firstConnection(false), m_reconnectTrack(0)
{
}

CSAssistReceiver::~CSAssistReceiver()
{
}

//--------------------------------------------------
#ifdef USE_UDP_LIBRARY
void CSAssistReceiver::OnConnectComplete(UdpConnection *con)
#else
void CSAssistReceiver::OnConnectRequest(TcpConnection *con)
#endif
//
// Called when API has successfully connected to server...
//--------------------------------------------------
{
	//fprintf(stderr, "OnConnectComplete: enter!\n");

	if (m_api->m_useRedirectServer)
	{
		switch(m_api->m_connectState)
		{
			case CONNECT_REDIRECT_NEGOTIATING: 
				m_api->m_connectState = CONNECT_REDIRECT_CONNECTED; 
				//if (m_firstConnection) 
					m_api->ConnectLB();
				break;
			case CONNECT_BACKEND_NEGOTIATING: 
				m_api->m_connectState = CONNECT_BACKEND_CONNECTED;
				//m_firstConnection = true; 
				break;
			default:
				fprintf(stderr, "OnConnectComplete(): unknown connect state (%u)\n", m_api->m_connectState);
		}
	}
	else
	{
		m_api->m_connectState = CONNECT_BACKEND_CONNECTED;
		//m_firstConnection = true;
	}
	//fprintf(stderr, "debug: OnConnectComplete(): Now m_connectState=%u\n", m_api->m_connectState);

	// resend identity to server
	if ((m_firstConnection == true || m_api->m_connectFailed)  && m_api->m_connectState == CONNECT_BACKEND_CONNECTED)
	{
		//fprintf(stderr, "Trying to auto re-connect\n");
		m_reconnectTrack = m_api->connectCSAssistInternal();
	}

}

#ifdef USE_UDP_LIBRARY
void CSAssistReceiver::OnTerminated(UdpConnection *con)
#else
void CSAssistReceiver::OnTerminated(TcpConnection *con)
#endif
{
	//fprintf(stderr,"OnTerminated(): enter\n");
	if (m_api->m_useRedirectServer)
	{
		m_api->m_connectReqCount = 0;
		switch(m_api->m_connectState)
		{
			case CONNECT_REDIRECT_CONNECTED:
			case CONNECT_REDIRECT_NEGOTIATING: 
				m_api->m_connectState = CONNECT_REDIRECT_DISCONNECTED; 
				m_api->GetLBHost();
				break;

			case CONNECT_BACKEND_CONNECTED_AND_AUTHED:
				m_api->m_api->OnDisconnectCSAssist(0, CSASSIST_RESULT_SERVER_DISCONNECT, nullptr);
			case CONNECT_BACKEND_CONNECTED:
				m_api->GetLBHost();
			case CONNECT_BACKEND_NEGOTIATING: m_api->m_connectState = CONNECT_BACKEND_DISCONNECTED; break;
			default:
				fprintf(stderr,"OnTermintated(): bad connectState (%u)\n", m_api->m_connectState);
		}
	}
	else
	{
		if (m_api->m_connectState == CONNECT_BACKEND_CONNECTED_AND_AUTHED)
			m_api->m_api->OnDisconnectCSAssist(0, CSASSIST_RESULT_SERVER_DISCONNECT, nullptr);
		m_api->m_connectState = CONNECT_BACKEND_DISCONNECTED;
		m_api->GetLBHost();
	}
}

#ifdef USE_UDP_LIBRARY
void CSAssistReceiver::OnCrcReject(UdpConnection *con, uchar *data, int dataLen)
#else
void CSAssistReceiver::OnCrcReject(TcpConnection *con, uchar *data, int dataLen)
#endif
{
}

//--------------------------------------------------
#ifdef USE_UDP_LIBRARY
void CSAssistReceiver::OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen)
#else
void CSAssistReceiver::OnRoutePacket(TcpConnection *con, const uchar *data, int dataLen)
#endif
//--------------------------------------------------
{
	unsigned track, result;
	short type;
	Response *res = 0;

	ByteStream msg(data, dataLen);
	ByteStream::ReadIterator iter = msg.begin();
	iter.advance(2);	// skip short num_args
	get(iter, type);
	get(iter, track);
	get(iter, result);

	if (type > CSASSIST_CALL_START_SERVER_MESSAGES)
	{			// ----- server-initiated calls need to create a new Response object -----
		res = m_api->createServerResponse(type);
	}
	else if (type == CSASSIST_CALL_CONNECTLB || (type == CSASSIST_CALL_CONNECT && m_firstConnection))
	{
		//fprintf(stderr,"OnRoutePacket(): got an internal connect/lb request\n");
		res = m_api->getPendingInt(track);
	}
	else		// ----- API-initiated calls already have a Response object -----
	{
		res = m_api->getPending(track);
	}
	if(res != nullptr)
	{
		res->init(type, track, result);
		res->decode(iter);
		m_api->CSAssistGameCallback(res);
	}
	else
	{
		//fprintf(stderr,"Dropping timed-out response type(%u) track(%u) result(%u)", type, track, result);
	}
}


} // namespace CSAssist
