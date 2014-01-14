// ======================================================================
//
// UdpConnectionHandlerMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpConnectionHandlerMT.h"
#include "UdpConnectionMT.h"
#include "UdpLibraryMT.h"
#include "Events.h"

// ======================================================================

UdpConnectionHandlerMT::UdpConnectionHandlerMT() :
	m_refCount(1)
{
}

// ----------------------------------------------------------------------

UdpConnectionHandlerMT::~UdpConnectionHandlerMT()
{
	DEBUG_FATAL (m_refCount != 0, ("UdpConnectionHandlerMT dtor called with nonzero refcount"));
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerMT::AddRef()
{
	++m_refCount;
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerMT::Release()
{
	if (--m_refCount == 0)
		delete this;
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerMT::OnConnectComplete(UdpConnectionMT *)
{
}

// ----------------------------------------------------------------------

void UdpConnectionHandlerMT::OnTerminated(UdpConnectionMT *)
{
}

//-----------------------------------------------------------------------

void UdpConnectionHandlerMT::OnPacketCorrupt(UdpConnectionMT *con, const uchar *data, int dataLen, UdpCorruptionReason reason)
{
	UNREF(con);
	UNREF(data);
	UNREF(dataLen);
	UNREF(reason);
	con->Disconnect();
}

// ======================================================================

