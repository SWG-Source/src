// ======================================================================
//
// SwgGameServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgGameServer_H
#define INCLUDED_SwgGameServer_H

#include "serverGame/GameServer.h"

class SwgGameServer : public GameServer
{
public:

	virtual ~SwgGameServer();

	static void install();

	virtual void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	
	virtual void handleCSRequest( GameServerCSRequestMessage & request );

protected:
	             SwgGameServer();
	virtual void initialize   ();
	virtual void shutdown     ();

private:
	SwgGameServer              (const SwgGameServer & source);
	SwgGameServer & operator = (const SwgGameServer & rhs);
};


#endif
