// TaskConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskConnection_H
#define	_INCLUDED_TaskConnection_H

//-----------------------------------------------------------------------

#include <algorithm>
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/Connection.h"

namespace Archive
{
	class ByteStream;
}

class GameNetworkMessage;
class TaskConnection;
class TaskHandler;

//-----------------------------------------------------------------------
struct Closed       {};
struct Failed       {};
struct Identified   {TaskConnection * connection; unsigned char id;};
struct Opened       {};
struct Overflowing  {unsigned int bytesPending;};
struct Receive      {const Archive::ByteStream * message;};
struct Reset        {};

class TaskConnection : public Connection
{
public:

public:
	TaskConnection(const std::string & remoteAddress, const unsigned short remotePort);
	TaskConnection(UdpConnectionMT *, TcpClient *);
	~TaskConnection();

	static int     getConnectionCount      ();
	virtual void   onConnectionClosed      ();
	virtual void   onConnectionOpened      ();
	virtual void   onConnectionOverflowing (const unsigned int bytesPending);
	virtual void   onReceive               (const Archive::ByteStream & message);
	void           send                    (const GameNetworkMessage & message); //lint !e1411 Member with different signature hides virtual member 'Connection::send(const Archive::ByteStream &, bool) const // jrandall how can it hide the virtual if the signature differs?

	MessageDispatch::Transceiver<const Closed &> &       getTransceiverClosed       ();
	MessageDispatch::Transceiver<const Failed &> &       getTransceiverFailed       ();
	MessageDispatch::Transceiver<const Identified &> &   getTransceiverIdentified   ();
	MessageDispatch::Transceiver<const Opened &> &       getTransceiverOpened       ();
	MessageDispatch::Transceiver<const Overflowing &> &  getTransceiverOverflowing  ();
	MessageDispatch::Transceiver<const Receive &> &      getTransceiverReceive      ();
	MessageDispatch::Transceiver<const Reset &> &        getTransceiverReset        ();
	
private: // capabilities / emissions
	MessageDispatch::Transceiver<const Closed &>       closed;
	MessageDispatch::Transceiver<const Failed &>       failed;
	MessageDispatch::Transceiver<const Identified &>   identified;
	MessageDispatch::Transceiver<const Opened &>       opened;
	MessageDispatch::Transceiver<const Overflowing &>  overflowing;
	MessageDispatch::Transceiver<const Receive &>      receiveMessage;
	MessageDispatch::Transceiver<const Reset &>        reset;

private:
	TaskConnection & operator = (const TaskConnection & rhs);
	TaskConnection(const TaskConnection & source);

private:
	TaskHandler *  handler;
}; //lint !e1712 default constructor not defined for class 'TaskConnection'

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskConnection_H
