// ======================================================================
//
// Events.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "Events.h"
#include "UdpHandlerMT.h"
#include "sharedSynchronization/Guard.h"

// ======================================================================

static unsigned char *s_outgoingEventData;
static int s_outgoingEventSize;
static int s_outgoingEventMax;
static unsigned char *s_incomingEventData;
static int s_incomingEventSize;
static int s_incomingEventMax;

// ======================================================================

inline int padEventLength(int length)
{
	return (length+15)&(~15);
}

// ======================================================================

static void growIncoming(int amount)
{
	int newSize = s_incomingEventSize + amount;
	if (newSize > s_incomingEventMax)
	{
		int newMax = s_incomingEventMax*2;
		while (newSize > newMax)
			newMax *= 2;
		unsigned char *newData = new unsigned char[newMax];
		memcpy(newData, s_incomingEventData, s_incomingEventSize);
		delete [] s_incomingEventData;
		s_incomingEventData = newData;
		s_incomingEventMax = newMax;
	}
}

// ----------------------------------------------------------------------

static void growOutgoing(int amount)
{
	int newSize = s_outgoingEventSize + amount;
	if (newSize > s_outgoingEventMax)
	{
		int newMax = s_outgoingEventMax*2;
		while (newSize > newMax)
			newMax *= 2;
		unsigned char *newData = new unsigned char[newMax];
		memcpy(newData, s_outgoingEventData, s_outgoingEventSize);
		delete [] s_outgoingEventData;
		s_outgoingEventData = newData;
		s_outgoingEventMax = newMax;
	}
}

// ======================================================================

void Events::install()
{
	s_incomingEventMax = 1024*1024;
	s_incomingEventData = new unsigned char[s_incomingEventMax];
	s_outgoingEventMax = 1024*1024;
	s_outgoingEventData = new unsigned char[s_outgoingEventMax];
}

// ----------------------------------------------------------------------

void Events::remove()
{
	delete [] s_incomingEventData;
	s_incomingEventData = 0;
	delete [] s_outgoingEventData;
	s_outgoingEventData = 0;
}

// ----------------------------------------------------------------------

void Events::processIncoming()
{
	int pos = 0;
	while (pos < s_incomingEventSize)
	{
		EventBase *event = reinterpret_cast<EventBase*>(s_incomingEventData+pos);
		switch (event->getType())
		{
		case ET_Receive:
			reinterpret_cast<EventReceive *>(event)->process(s_incomingEventData+pos+sizeof(EventReceive));
			break;
		case ET_ConnectComplete:
			reinterpret_cast<EventConnectComplete *>(event)->process();
			break;
		case ET_ConnectRequest:
			reinterpret_cast<EventConnectRequest *>(event)->process();
			break;
		case ET_Terminated:
			reinterpret_cast<EventTerminated *>(event)->process();
			break;
		default:
			FATAL(true, ("Unknown incoming event type"));
			break;
		}
		pos += event->getLength();
	}
	s_incomingEventSize = 0;
}

// ----------------------------------------------------------------------

void Events::processOutgoing()
{
	int pos = 0;
	while (pos < s_outgoingEventSize)
	{
		EventBase *event = reinterpret_cast<EventBase*>(s_outgoingEventData+pos);
		switch (event->getType())
		{
		case ET_SendRaw:
			reinterpret_cast<EventSendRaw *>(event)->process(s_outgoingEventData+pos+sizeof(EventSendRaw));
			break;
		case ET_SendLogicalPacket:
			reinterpret_cast<EventSendLogicalPacket *>(event)->process();
			break;
		default:
			FATAL(true, ("Unknown incoming event type"));
			break;
		}
		pos += event->getLength();
	}
	s_outgoingEventSize = 0;
}

// ----------------------------------------------------------------------

void Events::pushIncomingEventReceive(UdpConnectionMT *udpConnectionMT, unsigned char const *data, int dataLen)
{
	int length = padEventLength(sizeof(EventReceive)+dataLen);
	growIncoming(length);
	new(s_incomingEventData+s_incomingEventSize) EventReceive(udpConnectionMT, dataLen);
	memcpy(s_incomingEventData+s_incomingEventSize+sizeof(EventReceive), data, dataLen);
	s_incomingEventSize += length;
}

// ----------------------------------------------------------------------

void Events::pushIncomingEventConnectComplete(UdpConnectionMT *udpConnectionMT)
{
	int length = padEventLength(sizeof(EventConnectComplete));
	growIncoming(length);
	new(s_incomingEventData+s_incomingEventSize) EventConnectComplete(udpConnectionMT);
	s_incomingEventSize += length;
}

// ----------------------------------------------------------------------

void Events::pushIncomingEventConnectRequest(UdpManagerHandlerMT *udpManagerHandlerMT, UdpConnection *udpConnection)
{
	int length = padEventLength(sizeof(EventConnectRequest));
	growIncoming(length);
	new(s_incomingEventData+s_incomingEventSize) EventConnectRequest(udpManagerHandlerMT, udpConnection);
	s_incomingEventSize += length;
}

// ----------------------------------------------------------------------

void Events::pushIncomingEventTerminated(UdpConnectionMT *udpConnectionMT)
{
	int length = padEventLength(sizeof(EventTerminated));
	growIncoming(length);
	new(s_incomingEventData+s_incomingEventSize) EventTerminated(udpConnectionMT);
	s_incomingEventSize += length;
}

// ----------------------------------------------------------------------

void Events::pushOutgoingEventSendRaw(UdpConnection *udpConnection, UdpChannel udpChannel, unsigned char const *data, int dataLen)
{
	Guard lock(UdpLibraryMT::getMutex());
	int length = padEventLength(sizeof(EventSendRaw)+dataLen);
	growOutgoing(length);
	new(s_outgoingEventData+s_outgoingEventSize) EventSendRaw(udpConnection, udpChannel, dataLen);
	memcpy(s_outgoingEventData+s_outgoingEventSize+sizeof(EventSendRaw), data, dataLen);
	s_outgoingEventSize += length;
}

// ----------------------------------------------------------------------

void Events::pushOutgoingEventSendLogicalPacket(UdpConnection *udpConnection, UdpChannel udpChannel, LogicalPacket const *logicalPacket)
{
	Guard lock(UdpLibraryMT::getMutex());
	int length = padEventLength(sizeof(EventSendLogicalPacket));
	growOutgoing(length);
	new(s_outgoingEventData+s_outgoingEventSize) EventSendLogicalPacket(udpConnection, udpChannel, logicalPacket);
	s_outgoingEventSize += length;
}

// ======================================================================

EventBase::EventBase(EventType eventType, int length) :
	m_eventType(eventType),
	m_length(padEventLength(length))
{
}

// ======================================================================

EventReceive::EventReceive(UdpConnectionMT *udpConnectionMT, int dataLen) :
	EventBase(ET_Receive, sizeof(EventReceive)+dataLen),
	m_udpConnectionMT(udpConnectionMT),
	m_dataLen(dataLen)
{
	udpConnectionMT->AddRef();
}

// ----------------------------------------------------------------------

void EventReceive::process(unsigned char const *data)
{
	m_udpConnectionMT->processReceive(data, m_dataLen);
	m_udpConnectionMT->Release();
}

// ======================================================================

EventConnectComplete::EventConnectComplete(UdpConnectionMT *udpConnectionMT) :
	EventBase(ET_ConnectComplete, sizeof(EventConnectComplete)),
	m_udpConnectionMT(udpConnectionMT)
{
	udpConnectionMT->AddRef();
}

// ----------------------------------------------------------------------

void EventConnectComplete::process()
{
	m_udpConnectionMT->processConnectComplete();
	m_udpConnectionMT->Release();
}
	
// ======================================================================

EventConnectRequest::EventConnectRequest(UdpManagerHandlerMT *udpManagerHandlerMT, UdpConnection *udpConnection) :
	EventBase(ET_ConnectRequest, sizeof(EventConnectRequest)),
	m_udpManagerHandlerMT(udpManagerHandlerMT),
	m_udpConnectionMT(0)
{
	udpManagerHandlerMT->AddRef();
	udpConnection->AddRef();
	m_udpConnectionMT = new UdpConnectionMT(udpConnection);
}

// ----------------------------------------------------------------------

void EventConnectRequest::process()
{
	m_udpManagerHandlerMT->OnConnectRequest(m_udpConnectionMT);
	m_udpConnectionMT->Release();
	m_udpManagerHandlerMT->Release();
}

// ======================================================================

EventTerminated::EventTerminated(UdpConnectionMT *udpConnectionMT) :
	EventBase(ET_Terminated, sizeof(EventTerminated)),
	m_udpConnectionMT(udpConnectionMT)
{
	udpConnectionMT->AddRef();
}

// ----------------------------------------------------------------------

void EventTerminated::process()
{
	m_udpConnectionMT->processTerminated();
	m_udpConnectionMT->Release();
}

// ======================================================================

EventSendRaw::EventSendRaw(UdpConnection *udpConnection, UdpChannel udpChannel, int dataLen) :
	EventBase(ET_SendRaw, sizeof(EventSendRaw)+dataLen),
	m_udpConnection(udpConnection),
	m_udpChannel(udpChannel),
	m_dataLen(dataLen)
{
	udpConnection->AddRef();
}

// ----------------------------------------------------------------------

void EventSendRaw::process(unsigned char const *data)
{
	m_udpConnection->Send(m_udpChannel, data, m_dataLen);
	m_udpConnection->Release();
}

// ======================================================================

EventSendLogicalPacket::EventSendLogicalPacket(UdpConnection *udpConnection, UdpChannel udpChannel, LogicalPacket const *logicalPacket) :
	EventBase(ET_SendLogicalPacket, sizeof(EventSendLogicalPacket)),
	m_udpConnection(udpConnection),
	m_udpChannel(udpChannel),
	m_logicalPacket(logicalPacket)
{
	udpConnection->AddRef();
	logicalPacket->AddRef();
}

// ----------------------------------------------------------------------

void EventSendLogicalPacket::process()
{
	m_udpConnection->Send(m_udpChannel, m_logicalPacket);
	m_logicalPacket->Release();
	m_udpConnection->Release();
}

// ======================================================================

