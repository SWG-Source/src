// ======================================================================
//
// UdpConnectionHandlerMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpConnectionHandlerMT_H_
#define _UdpConnectionHandlerMT_H_

// ======================================================================

#include "UdpLibrary.h"

// ======================================================================

class UdpConnectionMT;

// ======================================================================

class UdpConnectionHandlerMT
{
public:
	UdpConnectionHandlerMT();

	void AddRef();
	void Release();

	virtual void OnRoutePacket(UdpConnectionMT *con, unsigned char const *data, int dataLen) = 0;
	virtual void OnConnectComplete(UdpConnectionMT *con);
	virtual void OnTerminated(UdpConnectionMT *con);
	virtual void OnPacketCorrupt(UdpConnectionMT *con, const uchar *data, int dataLen, UdpCorruptionReason reason);

private:
	UdpConnectionHandlerMT(UdpConnectionHandlerMT const &);
	UdpConnectionHandlerMT &operator=(UdpConnectionHandlerMT const &);

protected:
	virtual ~UdpConnectionHandlerMT();

private:
	int m_refCount;
};

// ======================================================================

#endif // _UdpConnectionHandlerMT_H_

