// ======================================================================
//
// Events.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _Events_H_
#define _Events_H_

// ======================================================================

#include "UdpLibraryMT.h"
#include "UdpManagerHandlerMT.h"

// ======================================================================

enum EventType
{
	// incoming event types
	ET_ConnectComplete,
	ET_ConnectRequest,
	ET_Receive,
	ET_Terminated,
	// outgoing event types
	ET_SendLogicalPacket,
	ET_SendRaw
};

// ======================================================================

class Events
{
public:
	static void install();
	static void remove();

	static void processIncoming();
	static void processOutgoing();

	static void pushIncomingEventReceive(UdpConnectionMT *udpConnectionMT, unsigned char const *data, int dataLen);
	static void pushIncomingEventConnectComplete(UdpConnectionMT *udpConnectionMT);
	static void pushIncomingEventConnectRequest(UdpManagerHandlerMT *udpManagerHandlerMT, UdpConnection *udpConnection);
	static void pushIncomingEventTerminated(UdpConnectionMT *udpConnectionMT);
	static void pushOutgoingEventSendRaw(UdpConnection *udpConnection, UdpChannel udpChannel, unsigned char const *data, int dataLen);
	static void pushOutgoingEventSendLogicalPacket(UdpConnection *udpConnection, UdpChannel udpChannel, LogicalPacket const *logicalPacket);
};

// ======================================================================

class EventBase
{
public:
	EventBase(EventType eventType, int length);

	EventType getType() const;
	int getLength() const;

private:
	EventType m_eventType;
	int m_length;
};

// ----------------------------------------------------------------------

inline EventType EventBase::getType() const
{
	return m_eventType;
}

// ----------------------------------------------------------------------

inline int EventBase::getLength() const
{
	return m_length;
}

// ======================================================================

class EventReceive: public EventBase
{
public:
	EventReceive(UdpConnectionMT *udpConnectionMT, int dataLen);
	void process(unsigned char const *data);

private:
	UdpConnectionMT *m_udpConnectionMT;
	int m_dataLen;
};

// ======================================================================

class EventConnectComplete: public EventBase
{
public:
	EventConnectComplete(UdpConnectionMT *udpConnectionMT);
	void process();

private:
	UdpConnectionMT *m_udpConnectionMT;
};

// ======================================================================

class EventConnectRequest: public EventBase
{
public:
	EventConnectRequest(UdpManagerHandlerMT *udpManagerHandlerMT, UdpConnection *udpConnection);
	void process();

private:
	UdpManagerHandlerMT *m_udpManagerHandlerMT;
	UdpConnectionMT *m_udpConnectionMT;
};

// ======================================================================

class EventTerminated: public EventBase
{
public:
	EventTerminated(UdpConnectionMT *udpConnectionMT);
	void process();

private:
	UdpConnectionMT *m_udpConnectionMT;
};

// ======================================================================

class EventSendRaw: public EventBase
{
public:
	EventSendRaw(UdpConnection *udpConnection, UdpChannel udpChannel, int dataLen);
	void process(unsigned char const *data);

private:
	UdpConnection *m_udpConnection;
	UdpChannel m_udpChannel;
	int m_dataLen;
};

// ======================================================================

class EventSendLogicalPacket: public EventBase
{
public:
	EventSendLogicalPacket(UdpConnection *udpConnection, UdpChannel udpChannel, LogicalPacket const *logicalPacket);
	void process();

private:
	UdpConnection *m_udpConnection;
	UdpChannel m_udpChannel;
	LogicalPacket const *m_logicalPacket;
};

// ======================================================================

#endif // _Events_H_

