#ifndef	_SERVER_SERVICE_HANDLER_H
#define	_SERVER_SERVICE_HANDLER_H

#include "Network/ConnectionHandler.h"

#include <list>
#include "ServiceHandler.h"

class ServerServiceHandler :
public ServiceHandler
{
public:
	virtual 		~ServerServiceHandler();
	virtual void	onConnectionClose(NetConnection * closingConnection, ConnectionHandler * connectionHandler, uint32 causeCode);
protected:
	std::list<ConnectionHandler *>	_connections;
};

inline ServerServiceHandler::~ServerServiceHandler()
{
	std::list<ConnectionHandler *>::iterator	i;
	ConnectionHandler *	c;
	
	for(i = _connections.begin(); i != _connections.end(); ++i)
	{
		c = *i;
		delete c;
	}
	_connections.clear();
}

inline void ServerServiceHandler::onConnectionClose(NetConnection * closingConnection, ConnectionHandler * connectionHandler, uint32 causeCode)
{
	_connections.remove(connectionHandler);
	delete connectionHandler;
}

#endif	_SERVER_SERVICE_HANDLER_H