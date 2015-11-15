// ======================================================================
//
// UdpManagerMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpManagerMT_H_
#define _UdpManagerMT_H_

// ======================================================================

#include "UdpLibrary.h"

// ======================================================================

class UdpConnectionMT;

// ======================================================================

class UdpManagerMT
{
public:
	typedef UdpManager::Params Params;

public:
	UdpManagerMT(UdpManager::Params *params);

	void AddRef();
	void Release();

	void GiveTime();

	UdpConnectionMT *EstablishConnection(char const *serverAddress, int serverPort, int timeout = 0);
	LogicalPacket const *CreatePacket(void const *data, int dataLen, void const *data2 = 0, int dataLen2 = 0);
	void ClearHandler();
	int GetLocalPort();

private:
	UdpManagerMT(UdpManagerMT const &);
	UdpManagerMT& operator=(UdpManagerMT const &);

	~UdpManagerMT();

private:
	int m_refCount;
	UdpManager *m_udpManager;
};

// ======================================================================

#endif // _UdpManagerMT_H_

