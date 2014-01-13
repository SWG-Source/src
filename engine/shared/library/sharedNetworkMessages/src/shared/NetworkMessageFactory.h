// NetworkMessageFactory.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_NetworkMessageFactory_H
#define	_INCLUDED_NetworkMessageFactory_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include <map>
#include "sharedNetwork/Connection.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "Singleton/Singleton.h"

class GameNetworkMessage;
class GameConnectionCallback;
class TcpClient;

template<typename MessageType> 
	inline unsigned long GetMessageId() 
{ 
	return 0; 
}

struct DispatchFunctorBase 
{ 
	virtual void dispatch(GameConnectionCallback *, GameNetworkMessage &) const = 0; 
};

//-----------------------------------------------------------------------
/**
	This class is responsible for creating GameNetworkMessage objects from
	Archive::ByteStream objects. It also handles dispatch of the message
	to target object members that are capable of and subscribed to the 
	message or connection/message pair.

	GameNetworkMessage construction happens via the makeMessage interface.
	makeMessage looks for the correpsonding construction function 
	(define by the MakeMessage(x) macro, yuck). MakeMessage instantiates
	a static struct with parmaters that identify 1) how to construct 
	a message, 2) how to identify a message for transceiver lookups and
	3) how to dispatch a message. This data is storedin the 
	NetworkMessageFactory singleton for later use by recivers and emitters.

	Dispatching is initiated by a GameConnectionCallback (or a derivative)
	passing itself and a byte stream to dispatchMessage(). Getting the 
	transceiver responsible for emitting the message is a constant time
	operation.

*/
class NetworkMessageFactory : public Singleton<NetworkMessageFactory>
{
public:
	~NetworkMessageFactory();

	void dispatchMessage(GameConnectionCallback * source, Archive::ReadIterator &);

	GameNetworkMessage * makeMessage(Archive::ReadIterator & bs);

protected:
	friend struct NetworkMessageMaker;
	friend class Singleton<NetworkMessageFactory>;
	NetworkMessageFactory();
	void registerMaker(unsigned long int, GameNetworkMessage *(*)(Archive::ReadIterator &), const DispatchFunctorBase *);

private:
	NetworkMessageFactory & operator = (const NetworkMessageFactory & rhs);
	NetworkMessageFactory(const NetworkMessageFactory & source);

private:
	std::map<unsigned long int, GameNetworkMessage * (*)(Archive::ReadIterator &)> messageMakers;
	std::map<unsigned long int, const DispatchFunctorBase *> dispatchers;
};

//-----------------------------------------------------------------------
/**
	The NetworkMessageMaker facilitates static registration of factory
	functions for GameNetworkMessage objects. Only GameNetworkMessage
	objects that are included in an application create an instance
	of the NetworkMessageMaker (no data members).

	On construction, the NetworkMessageMaker registers create and
	dispatch functions with the NetworkMessageFactory singleton.
*/
struct NetworkMessageMaker
{
	NetworkMessageMaker(unsigned long, GameNetworkMessage *(*maker)(Archive::ReadIterator &), const DispatchFunctorBase *);
	~NetworkMessageMaker();
};

template<typename MessageType>
struct NetworkMessage
{
	NetworkMessage(GameConnectionCallback & c, MessageType & m) : connection(c), message(m){};
	GameConnectionCallback & connection;
	MessageType & message;
	NetworkMessage & operator = (const NetworkMessage &);
};

//-----------------------------------------------------------------------
/**
	Generates a unique unsigned long integer from a character string. The
	id is unique within the scope of the current application for the 
	current run ONLY. Ids are generated sequentially, to better support
	storing maps of ids to other types in vectors rather than maps.

	This is intended to be used during type initialization (usually 
	during application startup or the first time a new type id is
	required). It is a logN operation (map lookup). 
*/
unsigned long GetNewRuntimeMessageId(const char * messageName);

//-----------------------------------------------------------------------
/**
	GameConnectionCallback sits between Network::ConnectionCallback
	and the game to handle message dispatching.

	It implements onReceiveMessage(Archive::ReadIterator &)
	and invokes NetworkMessageFactory::dispatchMessage(). This dispatch
	processes uses the sharedMessageDispatch system's Transceiver
	to deliver messages to target object member functions. 

	Dispatchers are (currently) organized in a vector. The messageId
	is determined from the type of message, which is incrementally
	defined as each new message type comes online. This is unique and
	consistent only for the current run of the application. These
	ID's CAN NOT be persisted or used between separate invocations
	of the application!
*/
class GameConnectionCallback : public Connection
{
public:
	GameConnectionCallback(UdpConnectionMT *, TcpClient *);
	virtual ~GameConnectionCallback();

	template<typename MessageType, typename ObjectType>
		void listenForMessage(MessageDispatch::Callback & cb, ObjectType & object, void (ObjectType::*callback)(MessageType) )
	{
		unsigned long messageId = GetMessageId<MessageType>();
		MessageDispatch::Transceiver<MessageType> * t = static_cast<MessageDispatch::Transceiver<MessageType> *>(getTransceiver(messageId));
		if(!t)
		{
			t = new MessageDispatch::Transceiver<MessageType>;
			addTransceiver(messageId, t);
		}
		cb.connect(*t, object, callback);
	}

	void                                addTransceiver  (const unsigned long messageId, MessageDispatch::TransceiverBase * newTransceiver);
	MessageDispatch::TransceiverBase *  getTransceiver  (const unsigned long messageId);
	virtual void                        onReceive       (const Archive::ByteStream & message);

private:
	std::vector<MessageDispatch::TransceiverBase *> transceivers;
	GameConnectionCallback(const GameConnectionCallback &);
	GameConnectionCallback & operator = (const GameConnectionCallback &);
};

//-----------------------------------------------------------------------

/**
	Yes, this is in fact a macro. It would be a template if I could
	figure out how to do it :) 

	It defines a clone/factory function, registers the function by way
	of a static instance of a NetworkMessageMaker, and defines a dispatch
	routine which is also registered (again via the static 
	NetworkMessageMaker). When a byte stream is passed to the 
	NetworkMessageFactory singleton, it reconstructs the appropriate
	game network message. 
	
	A GameConnectionCallback invokes 
	NetworkMessageFactory::dispatch() with the byte stream. The
	factory creates a GameNetworkMessage and passes it to it's 
	corresponding dispatch function. The dispatch function casts
	it to the right type, finds a transceiver for the type (or
	creates one if it doesn't yet exist), and emits the message.

	All transceivers listening for a NetworkMessage<GameMessageType> 
	from the source GameConnectionCallback object or callbacks listening
	anonymously will receive the message.
*/


template<typename MessageType>
struct DispatchFunctor : public DispatchFunctorBase
{
	void dispatch(GameConnectionCallback * connection, GameNetworkMessage & base) const
	{
		MessageType & message = static_cast<MessageType &>(base);
		NetworkMessage<MessageType> m(*connection, message);
		unsigned long typeId = GetMessageId<MessageType>();
		MessageDispatch::Transceiver<NetworkMessage<MessageType> &> * t = static_cast<MessageDispatch::Transceiver<NetworkMessage<MessageType> &> *>(connection->getTransceiver(typeId));
		if(!t)
		{
			t = new MessageDispatch::Transceiver<NetworkMessage<MessageType> &>;
			connection->addTransceiver(typeId, t);
		}
		t->emitMessage(m);
	}
};

#define MakeMessage(x) inline GameNetworkMessage * make##x(Archive::ReadIterator & source) { \
	return new x ( source ) ; \
	} \
    template<> inline unsigned long GetMessageId<x>() \
	{ \
		static const unsigned long id = GetNewRuntimeMessageId(#x); \
		return id; \
	} \
	static const DispatchFunctor<x> dispatch##x; \
	static const NetworkMessageMaker maker##x(MessageDispatch::MessageBase::makeMessageTypeFromString(#x), make##x, &dispatch##x); 

//-----------------------------------------------------------------------

#endif	// _INCLUDED_NetworkMessageFactory_H
