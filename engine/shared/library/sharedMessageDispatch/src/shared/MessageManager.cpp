#include "sharedMessageDispatch/FirstSharedMessageDispatch.h"

#include "sharedDebug/Profiler.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedMessageDispatch/Receiver.h"

#include <set>
#include <unordered_map>

namespace MessageDispatch {

MessageManager MessageManager::ms_instance;

struct MessageManager::Data
{
	std::unordered_map<unsigned long int, std::set<Receiver *> > receivers;
	std::unordered_map<unsigned long int, std::set<void (*)(const Emitter &, const MessageBase &)> > staticCallbacks;
};

//---------------------------------------------------------------------
/**
	@brief Do NOT construct Singleton objects directly!
*/
MessageManager::MessageManager()
: data (new Data)
{
}

//---------------------------------------------------------------------
/**
	@brief Do NOT destroy Singleton objects directly!
*/
MessageManager::~MessageManager()
{
	delete data;
	data = 0;
}

//---------------------------------------------------------------------
/**
	@brief add a Receiver object to a map of Receiver objects that will
	receive all messages of a certain type.

	Some Receiver objects may wish to receive a message of a certain 
	type regardless of the Emitter object that originates the message.

	@param target          The Receiver object that will listen for all
	                       messages of the requested type
	@param messageTypeName The message type to listen for
	
	@see Emitter::addReceiver
	
	@author Justin Randall
*/
void MessageManager::addReceiver(Receiver & target, const char * const messageTypeName)
{
	const unsigned long int messageType = MessageBase::makeMessageTypeFromString(messageTypeName);
	addReceiver(target, messageType);
}

//-----------------------------------------------------------------------

void MessageManager::addReceiver(Receiver & target, const MessageBase & source)
{
	const unsigned long int messageType = source.getType();
	addReceiver(target, messageType);
}

//-----------------------------------------------------------------------

void MessageManager::addReceiver(Receiver & target, const unsigned long int messageType)
{
	std::unordered_map<unsigned long int, std::set<Receiver *> >::iterator i = data->receivers.find(messageType);
	if(i != data->receivers.end())
	{
		target.setHasTargets(true);
		std::set<Receiver *> & targets = (*i).second;
		targets.insert(&target);//lint !e534 // ignoring iterator returned from insert
	}
	else
	{
		target.setHasTargets(true);
		std::set<Receiver *> newTargets;
		newTargets.insert(&target);//lint !e534 // ignoring iterator returned from insert
		data->receivers[messageType] = newTargets;
	}
}

//-----------------------------------------------------------------------

void MessageManager::addStaticCallback(void (*callback)(const Emitter &, const MessageBase &), const unsigned long int messageType)
{
	std::unordered_map<unsigned long int, std::set<void (*)(const Emitter &, const MessageBase &)> >::iterator f = data->staticCallbacks.find(messageType);
	if(f != data->staticCallbacks.end())
	{
		std::set<void (*)(const Emitter &, const MessageBase &)> & targets = f->second;
		targets.insert(callback);
	}
	else
	{
		std::set<void (*)(const Emitter &, const MessageBase &)> newTargets;
		newTargets.insert(callback);
		data->staticCallbacks[messageType] = newTargets;
	}
}

//---------------------------------------------------------------------
/**
	@brief Invoked by Emitter objects when a message is broadcast

	@param emitter      The source of the MessageBase object being broadcast
	@param message      The message being broadcast

	@see Emitter::emit
	@see Receiver::onReceive

	@author Justin Randall
*/
void MessageManager::emitMessage(const Emitter & emitter, const MessageBase & message) const
{
	const unsigned long int messageType = message.getType();
	std::unordered_map<unsigned long int, std::set<Receiver *> >::const_iterator i = data->receivers.find(messageType);
	if(i != data->receivers.end())
	{
		const std::set<Receiver *> targets = (*i).second;
		std::set<Receiver *>::const_iterator j;
		for(j = targets.begin(); j != targets.end(); ++j)
		{
			Receiver * r = (*j);
			if(! emitter.hasReceiver(*r, messageType))
			{
				r->receiveMessage(emitter, message);
			}
		}
	}

	std::unordered_map<unsigned long int, std::set<void (*)(const Emitter &, const MessageBase &)> >::iterator f = data->staticCallbacks.find(messageType);
	if(f != data->staticCallbacks.end())
	{
		const std::set<void (*)(const Emitter &, const MessageBase &)> targets = f->second;
		std::set<void (*)(const Emitter &, const MessageBase &)>::const_iterator c;
		for(c = targets.begin(); c != targets.end(); ++c)
		{
			void (*callback)(const Emitter &, const MessageBase &) = *c;
			callback(emitter, message);
		}
	}
}

//---------------------------------------------------------------------
/**
	@brief Invoked by a Receiver during it's destructor
*/
void MessageManager::receiverDestroyed(const Receiver & target)
{
	if (!target.getHasTargets())
	{
		return;
	}
	// find receiver
	std::unordered_map<unsigned long int, std::set<Receiver *> >::iterator i;
	for(i = data->receivers.begin(); i != data->receivers.end(); ++i)
	{
		std::set<Receiver *> & targets = (*i).second;
		// const cast to satisfy STL semantics, target remains unchanged
		std::set<Receiver *>::iterator j = targets.find(const_cast<Receiver *>(&target));
		if(j != targets.end())
		{
			targets.erase(j);
		}
	}
}

//---------------------------------------------------------------------
/**
	@brief break a Receiver->Message relationship

	When a Receiver no longer wants to receive all messages of a certain
	type, it invokes MessageManager::removeReceiver to irradicate
	any relationship with the message in the MessageManager target map.

	@param target            The Receiver object that is breaking the 
	                         connection.
	@param messageTypeName   Identifies the source MessageBase objects
	                         that the Receiver object will now ignore.

	@see Emitter::removeReceiver

	@author Justin Randall
*/
void MessageManager::removeReceiver(const Receiver & target, const char * const messageTypeName)
{
	const unsigned long int messageType = MessageBase::makeMessageTypeFromString(messageTypeName);
	removeReceiver(target, messageType);
}

//-----------------------------------------------------------------------

void MessageManager::removeReceiver(const Receiver & target, const unsigned long int messageType)
{
	std::unordered_map<unsigned long int, std::set<Receiver *> >::iterator i = data->receivers.find(messageType);
	if(i != data->receivers.end())
	{
		std::set<Receiver *> & targets = (*i).second;
		std::set<Receiver *>::iterator j = targets.find(const_cast<Receiver *>(&target));
		if(j != targets.end())
		{
			targets.erase(j);
		}
	}
}

//-----------------------------------------------------------------------

void connectToMessage(const char * const messageTypeName, void (*callback)(const Emitter &, const MessageBase &))
{
	const unsigned long int messageType = MessageBase::makeMessageTypeFromString(messageTypeName);
	MessageManager::getInstance().addStaticCallback(callback, messageType);
}

//---------------------------------------------------------------------

}//namespace MessageDispatch
