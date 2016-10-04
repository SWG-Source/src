//---------------------------------------------------------------------

#include "sharedMessageDispatch/FirstSharedMessageDispatch.h"

#include "sharedDebug/Profiler.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Message.h"

#include <cassert>
#include <set>
#include <map>

namespace MessageDispatch {
struct Emitter::ReceiverList
{
	typedef std::set<Receiver *>                          ReceiverSet;
	typedef std::map<unsigned long int, ReceiverSet> Container;
	mutable Container c;
};

//---------------------------------------------------------------------
/**
	@brief Construct an emitter

	An Emitter is an abstract base. This is never directly invoked.

	@author Justin Randall
*/
Emitter::Emitter() :
receiverList(new ReceiverList)
{
	assert (receiverList != nullptr);
}

//---------------------------------------------------------------------
/**
	@brief pure virtual destructor

	The Emitter destructor cleans up message dispatch relationships
*/
Emitter::~Emitter()
{
	for(ReceiverList::Container::iterator i = receiverList->c.begin(); i != receiverList->c.end(); ++i)
	{
		ReceiverList::ReceiverSet & targets = (*i).second;		
		for(ReceiverList::ReceiverSet::iterator j = targets.begin(); j != targets.end(); ++j)
		{
			Receiver * r = (*j);
			r->emitterDestroyed(*this);
		}
	}

	delete receiverList;
	receiverList = 0;
}

//---------------------------------------------------------------------
/**
	@brief Attach a Receiver object directly to this Emitter

	When a Receiver needs to specifically target a particular emitter,
	it should invoke addReceiver and specify the message type it is 
	subscribing to.

	@param target          The Receiver object that will receive the 
	                       specified message when the Emitter emits.
	@param messageTypeName  The message type that the Emitter should 
	                        deliver to the target Receiver.

	@see Receiver
	@see MessageManager::addReceiver

	@author Justin Randall
*/
void Emitter::addReceiver(Receiver & target, const char * const messageTypeName) const
{
	const unsigned long int messageType = MessageBase::makeMessageTypeFromString(messageTypeName);
	addReceiver(target, messageType);
}

//-----------------------------------------------------------------------

void Emitter::addReceiver(Receiver & target, const MessageBase & source) const
{
	const unsigned long int messageType = source.getType();
	addReceiver(target, messageType);
}

//-----------------------------------------------------------------------

void Emitter::addReceiver(Receiver & target, const unsigned long int messageType) const
{
	ReceiverList::Container::iterator i = receiverList->c.find(messageType);

	if(i == receiverList->c.end())
	{
		ReceiverList::ReceiverSet newSet;
		newSet.insert(&target); //lint !e534 // ignoring iterator returned from insert
		receiverList->c.insert (std::make_pair (messageType, newSet));
	}
	else
	{
		ReceiverList::ReceiverSet & s = (*i).second;
		s.insert(&target); //lint !e534 // ignoring iterator returned from insert
	}
}

//---------------------------------------------------------------------
/**
	@brief emit a message

	All targeted Receivers will receive the Message that is emitted by 
	this Emitter object. The Message is forwarded to the MessageManager
	to be dispatched to objects that are interested in receiving ALL
	messages of a specific type, regardless of who emits it. Those
	Receiver objects will only receive the Message via the MessageManager
	if they did NOT receive the message from this Emitter object as a 
	targetted message.

	@param message   A const reference to a MessageBase object that will
	                 be delivered to registered Receivers.

	@see addReceiver
	@see MessageManager::addReceiver

	@author Justin Randall
*/
void Emitter::emitMessage(const MessageBase & message) const
{
	NOT_NULL(receiverList);
	
	ReceiverList::Container::iterator i = receiverList->c.find(message.getType());
	if(i != receiverList->c.end())
	{
		//-- make copy
		const ReceiverList::ReceiverSet targets = (*i).second;
		for(ReceiverList::ReceiverSet::const_iterator j = targets.begin(); j != targets.end(); ++j)
		{
			Receiver * r = (*j);
			NOT_NULL(r);
			r->receiveMessage(*this, message);
		}
	}
	MessageManager::getInstance().emitMessage(*this, message);
}

//---------------------------------------------------------------------
/**
	@brief helper to determine if an Emitter has a Receiver in it's target
	list

	This helper is invoked by the MessageManager to determine if a 
	Receiver is registered as a target for a message type. When the 
	Emitter emits a message, all Receiver objects registered with the
	Emitter will receive the message. If a Receiver object is ALSO registered
	with the MessageManager to receive ALL messages of the requested type,
	the MessageManager invokes this method on the source Emitter object
	to ensure that the Receiver target is not delivered the same message
	twice.

	@param target        The receiver to search for
	@param messageType   The MessageBase typeID of the message that the 
	                     receiver is interested in.

	@return true if this Emitter object has the Receiver mapped to the
	messageType in it's target list.

	@see MessageManager

	@author Justin Randall
*/
const bool Emitter::hasReceiver(const Receiver & target, const unsigned long int messageType) const
{
	bool result = false;
	ReceiverList::Container::const_iterator i = receiverList->c.find(messageType);
	if(i != receiverList->c.end())
	{
		const ReceiverList::ReceiverSet & targets = (*i).second;
		const ReceiverList::ReceiverSet::const_iterator j = targets.find(const_cast<Receiver *>(&target)); //stl is broken
		result = (j != targets.end());
	}
	return result;
}

//---------------------------------------------------------------------
/**
	@brief helper to determine if an Emitter has a Receiver in it's target
	list

	This helper is invoked to determine if a Receiver is registered as a
	target for any message type.

	@param target        The receiver to search for

	@return true if this Emitter object has the Receiver mapped to any messageType

	@see MessageManager

	@author John Watson
*/
const bool Emitter::hasReceiver(const Receiver & target) const
{
	for (ReceiverList::Container::const_iterator i = receiverList->c.begin (); i != receiverList->c.end (); ++i)
	{
		const ReceiverList::ReceiverSet & targets = (*i).second;
		const ReceiverList::ReceiverSet::const_iterator j = targets.find(const_cast<Receiver *>(&target)); //stl is broken
		if (j != targets.end())
			return true;
	}
	return false;
}

//---------------------------------------------------------------------
/**
	@brief remove a receiver from target lists

	This is most often invoked from a Receiver object's destructor. It
	removes the receiver from all Message mappings.

	@param target    The receiver that is being destroyed

	@author Justin Randall
*/
void Emitter::receiverDestroyed(const Receiver & target) const
{
	// find the receiver
	for(ReceiverList::Container::iterator i = receiverList->c.begin(); i != receiverList->c.end(); ++i)
	{
		ReceiverList::ReceiverSet & targets = (*i).second;
		const ReceiverList::ReceiverSet::iterator j = targets.find(const_cast<Receiver *>(&target)); //stl is broken

		if(j != targets.end())
		{
			targets.erase(j);
		}
	}
}

//---------------------------------------------------------------------
/**
	@brief Remove a Receiver object from a specific message in the target map

	@param target           The Receiver object that will break the 
	                        Target->message relationship.
	@param messageTypeName  A message type describing the message type the
	                        Receiver will disassociate itself from

	@author Justin Randall
*/
void Emitter::removeReceiver(const Receiver & target, const char * const messageTypeName) const
{
	unsigned long int messageType = MessageBase::makeMessageTypeFromString(messageTypeName);
	
	const ReceiverList::Container::iterator i = receiverList->c.find(messageType);
	if(i != receiverList->c.end())
	{
		ReceiverList::ReceiverSet & rset = (*i).second;
		//const cast for broken stl
		rset.erase(const_cast<Receiver *>(&target)); //lint !e534 // ignoring iterator returned from insert
	}
}

//---------------------------------------------------------------------

}//namespace MessageDispatch

