// NetworkMessageFactory.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "Archive/ByteStream.h"
#include "NetworkMessageFactory.h"

class TcpClient;

//-----------------------------------------------------------------------

unsigned long GetNewRuntimeMessageId(const char * messageName)
{
	unsigned long result;
	static std::map<const char *, unsigned long> ids;
	
	// find the string in the static map of string/id's.
	std::map<const char *, unsigned long>::const_iterator f = ids.find(messageName);

	if(f == ids.end())
	{
		// it's not there, assign a new id, increment the static nextId
		static unsigned long nextId = 0;
		ids[messageName] = nextId;
		result = nextId;
		nextId++;
	}
	else
	{
		// found an existing ID. Often happens during initialization
		// of statics declared in headers.
		result = (*f).second;
	}
	return result;		
}

//-----------------------------------------------------------------------

NetworkMessageMaker::NetworkMessageMaker(unsigned long typeId, GameNetworkMessage *(*maker)(Archive::ReadIterator &), const DispatchFunctorBase * dispatcher)
{
	NetworkMessageFactory::getInstance().registerMaker(typeId, maker, dispatcher);
}

//-----------------------------------------------------------------------

NetworkMessageMaker::~NetworkMessageMaker()
{
}

//-----------------------------------------------------------------------

NetworkMessageFactory::NetworkMessageFactory()
{
}

//-----------------------------------------------------------------------

NetworkMessageFactory::NetworkMessageFactory(const NetworkMessageFactory &)
{

}

//-----------------------------------------------------------------------

NetworkMessageFactory::~NetworkMessageFactory()
{
}

//-----------------------------------------------------------------------

GameConnectionCallback::GameConnectionCallback(UdpConnectionMT * u, TcpClient * t) :
Connection(u, t)
{
}

//-----------------------------------------------------------------------

GameConnectionCallback::~GameConnectionCallback()
{
	std::vector<MessageDispatch::TransceiverBase *>::iterator i;
	for(i = transceivers.begin(); i != transceivers.end(); ++i)
	{
		delete (*i);
	}
}

//-----------------------------------------------------------------------

void GameConnectionCallback::addTransceiver(const unsigned long messageId, MessageDispatch::TransceiverBase * newTransceiver)
{
	if(messageId >= transceivers.size())
		transceivers.resize(messageId +1);
	transceivers[messageId] = newTransceiver;
}

//-----------------------------------------------------------------------

MessageDispatch::TransceiverBase * GameConnectionCallback::getTransceiver(const unsigned long messageId)
{
	if(messageId >= transceivers.size())
		return 0;

	return transceivers[messageId];
}

//-----------------------------------------------------------------------

void GameConnectionCallback::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	NetworkMessageFactory::getInstance().dispatchMessage(this, r);
}

//-----------------------------------------------------------------------

NetworkMessageFactory & NetworkMessageFactory::operator = (const NetworkMessageFactory & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

void NetworkMessageFactory::dispatchMessage(GameConnectionCallback * connection, Archive::ReadIterator & source)
{
	// save a read iterator
	static Archive::ReadIterator r;
	r = source;

	// get message type
	GameNetworkMessage * msg = makeMessage(r);
	if(msg)
	{
		unsigned long t = msg->getType();
		
		std::map<unsigned long, const DispatchFunctorBase *>::iterator f = dispatchers.find(t);
		if(f != dispatchers.end())
		{
			(*f).second->dispatch(connection, *msg);
		}
	}
	delete msg;
}

//-----------------------------------------------------------------------

void NetworkMessageFactory::registerMaker(unsigned long typeId, GameNetworkMessage *(*maker)(Archive::ReadIterator &), const DispatchFunctorBase * dispatcher)
{
	messageMakers[typeId] = maker;
	dispatchers[typeId] = dispatcher;
}

//-----------------------------------------------------------------------

GameNetworkMessage * NetworkMessageFactory::makeMessage(Archive::ReadIterator & source)
{
	Archive::ReadIterator ri = source;
	GameNetworkMessage * result = 0;
	GameNetworkMessage msg(ri);
	unsigned long messageType = msg.getType();

	std::map<unsigned long, GameNetworkMessage *(*)(Archive::ReadIterator &)>::const_iterator f = messageMakers.find(messageType);
	if(f != messageMakers.end())
	{
		ri = source;
		result = (*f).second(ri);
	}
	return result;
}

//-----------------------------------------------------------------------
