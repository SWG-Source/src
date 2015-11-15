#ifndef __CSASSISTRECEIVER_H__
#define __CSASSISTRECEIVER_H__

//------------------------------------------------------------------------------
//
// CSAssistreceiver.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// UDPLibrary wrapper and handler
//
//------------------------------------------------------------------------------
#ifdef USE_UDP_LIBRARY
#include "UdpLibrary/UdpHandler.hpp"
#else
#include <TcpLibrary/TcpConnection.h>
#include <TcpLibrary/TcpHandlers.h>
#include <TcpLibrary/TcpManager.h>
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

namespace CSAssist
{

class CSAssistGameAPIcore;

#ifdef USE_UDP_LIBRARY
class CSAssistReceiver : public UdpConnectionHandler
#else
class CSAssistReceiver : public TcpConnectionHandler, public TcpManagerHandler
#endif
{
public:
	CSAssistReceiver(CSAssistGameAPIcore *api);
	virtual ~CSAssistReceiver();

#ifdef USE_UDP_LIBRARY
	virtual void OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
	virtual void OnConnectComplete(UdpConnection *con);
	virtual void OnTerminated(UdpConnection *con);
	virtual void OnCrcReject(UdpConnection *con, uchar *data, int dataLen);
#else
	virtual void OnRoutePacket(TcpConnection *con, const uchar *data, int dataLen);
	virtual void OnConnectRequest(TcpConnection *con);
	virtual void OnTerminated(TcpConnection *con);
	virtual void OnCrcReject(TcpConnection *con, uchar *data, int dataLen);
#endif

	CSAssistGameAPITrack getReconnectTrack()		{ return m_reconnectTrack; }

	friend class CSAssistGameAPIcore;
private:
	CSAssistGameAPIcore		*m_api;
	bool					m_firstConnection;
	CSAssistGameAPITrack	m_reconnectTrack;
};


} // namespace CSAssist

#endif

