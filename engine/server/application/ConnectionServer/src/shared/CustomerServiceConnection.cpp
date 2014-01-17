// CustomerServiceConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "CustomerServiceConnection.h"
#include "ClientConnection.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"

//-----------------------------------------------------------------------

CustomerServiceConnection::CustomerServiceConnection(UdpConnectionMT * u, TcpClient * t) :
		ServerConnection(u, t),
		clients()
{
}

//-----------------------------------------------------------------------

CustomerServiceConnection::~CustomerServiceConnection()
{
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::addClient(Client * newClient)
{
	if (clients.find(newClient) == clients.end())
		IGNORE_RETURN( clients.insert(newClient) );
	else
		DEBUG_WARNING(true, ("called CustomerServiceConnection::addClient with a client that already exists in the map."));
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CustomerServiceConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

const std::set<Client *> & CustomerServiceConnection::getClients() const
{
	return clients;
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("CustomerServiceConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
    GameNetworkMessage m(ri);
    ri = message.begin();

    if (m.isType("GameClientMessage"))
    {
        //we're receiving a message to forward to the client.
        //it is prefixed with NetworkId and reliable.
        const GameClientMessage msg(ri);
		Archive::ReadIterator mri(msg.getByteStream());
		GameNetworkMessage gnm(mri);
		mri = msg.getByteStream().begin();

		const std::vector<NetworkId> & d = msg.getDistributionList();
		std::vector<NetworkId>::const_iterator i;
		for(i = d.begin(); i != d.end(); ++i)
		{

			Client* client = ConnectionServer::getClient((*i));
			DEBUG_REPORT_LOG(!client, ("Error, could not map %s to a client\n", (*i).getValueString().c_str()));
			if (client)
			{
				client->getClientConnection()->sendByteStream(msg.getByteStream(), msg.getReliable());
			}
		}
	}
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::removeClient(Client * oldClient)
{
	std::set<Client *>::iterator f = clients.find(oldClient);
	if(f != clients.end())
		clients.erase(f);
}

//-----------------------------------------------------------------------


