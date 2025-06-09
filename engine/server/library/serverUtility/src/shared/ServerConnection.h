// ======================================================================
//
// ServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall
// ======================================================================

#ifndef	_ServerConnection_H
#define	_ServerConnection_H

// ======================================================================

#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedNetwork/Connection.h"

// ======================================================================

class GameNetworkMessage;

// ======================================================================

class ServerConnection: public Connection, public MessageDispatch::Emitter
{
public:
	          ServerConnection   (const std::string & address, const unsigned short port, const NetworkSetupData &);
	          ServerConnection   (UdpConnectionMT *, TcpClient * t);
	virtual   ~ServerConnection  ();

	static void                 install                 ();
	static const unsigned long  makeProcessId           ();
	static bool                 isMessageForwardable    (unsigned long int type);

	const uint32_t              getProcessId            () const;
	const uint32_t              getOsProcessId          () const;
	virtual void                onConnectionClosed      ();
	virtual void                onConnectionOpened      ();
	virtual void                onConnectionOverflowing (const unsigned int bytesPending);
	virtual void                onConnectionStalled     (const unsigned long stallTimeMs);
	virtual void	            onReceive               (const Archive::ByteStream & message);
	virtual void                reportReceive           (const Archive::ByteStream & bs);
	virtual void                reportSend              (const Archive::ByteStream & bs);
	virtual void                send                    (const GameNetworkMessage & message, const bool reliable);
	virtual void                setProcessId            (const uint32_t newProcessId);

public:
	class MessageConnectionCallback: public MessageDispatch::MessageBase
	{
	public:
		MessageConnectionCallback(const char * const messageName);
		~MessageConnectionCallback();
	};

	class MessageConnectionOverflowing : public MessageDispatch::MessageBase
	{
	public:
		MessageConnectionOverflowing(const unsigned int bytesPending);
		~MessageConnectionOverflowing();

		const unsigned int getBytesPending() const;
	private:
		unsigned int bytesPending;
	};

private:
	ServerConnection(const ServerConnection&); //disable
	ServerConnection &operator=(const ServerConnection&); //disable

private:
	uint32_t  processId;
	uint32_t  osProcessId; // remote's operating system assigned PID
};

//-----------------------------------------------------------------------

inline const uint32_t ServerConnection::getProcessId(void) const
{
	return processId;
}

//-----------------------------------------------------------------------

inline const uint32_t ServerConnection::getOsProcessId() const
{
	return osProcessId;
}

// ======================================================================

#endif	// _ServerConnection_H

