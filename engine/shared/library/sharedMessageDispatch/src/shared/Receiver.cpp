//---------------------------------------------------------------------
#include "sharedMessageDispatch/FirstSharedMessageDispatch.h"

#include "sharedDebug/Profiler.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/MessageManager.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"

#include <cassert>

#include <set>
#include <map>

namespace MessageDispatch {

struct Receiver::EmitterTargets
{
	typedef std::set<const Emitter *> Container;
	Container c;
};

//---------------------------------------------------------------------
/**
	@brief construct a Receiver object

	Receiver objects are abstract base classes. Do NOT instantiate
	a Receiver directly!

	@author Justin Randall
*/
Receiver::Receiver() :
emitterTargets(new EmitterTargets),
hasTargets(false)
{
	assert(emitterTargets != nullptr);
}

//---------------------------------------------------------------------

void Receiver::setHasTargets(bool targets)
{
	hasTargets = targets;
}

//---------------------------------------------------------------------

bool Receiver::getHasTargets() const
{
	return hasTargets;
}

//---------------------------------------------------------------------
/**
	@brief destroy the Receiver object

	Finds all Emitter objects that this Receiver is connected to and
	invokes Emitter::receiveMessagerDestroyed

	It then advises the MessageManager that the Receiver object is
	being destroyed.

	@author Justin Randall
*/
Receiver::~Receiver()
{
	disconnectAll();
	delete emitterTargets;
	emitterTargets = 0;
}

//---------------------------------------------------------------------
/**
	@brief connect to an Emitter to receive a message that it emits

	When an Emitter object emits a message, it dispatches the message
	to all subscribed receivers before sending it to the MessageManager
	for broadcasting.

	Receiver objects are inserted into the Emitter target map by way of
	this function (which wraps Emitter::addReceiver)

	@param target            The Emitter object to connect to
	@param messageTypeName   The message type to listen for

	@see Emitter::addReceiver
	@see MessageManager::addReceiver

	@author Justin Randall
*/
void Receiver::connectToEmitter(const Emitter & target, const char * const messageTypeName)
{
	target.addReceiver(*this, messageTypeName);
	emitterTargets->c.insert (&target);
}

//-----------------------------------------------------------------------
/**
	@brief advise the MessageManager that this Reciever object is 
	interested in ALL messages of the requested type.

	@param messageTypeName   A string describing the type of message

	@author Justin Randall
*/
void Receiver::connectToMessage(const char * const messageTypeName)
{
	MessageManager::getInstance().addReceiver(*this, messageTypeName);
}

//-----------------------------------------------------------------------

void Receiver::connectToMessage(const MessageBase & source)
{
	MessageManager::getInstance().addReceiver(*this, source);
}

//-----------------------------------------------------------------------

void Receiver::disconnectAll()
{
	EmitterTargets::Container::iterator i;
	for(i = emitterTargets->c.begin(); i != emitterTargets->c.end(); ++i)
	{
		const Emitter * e = (*i);
		e->receiverDestroyed(*this);
	}
	emitterTargets->c.clear();
	MessageManager::getInstance().receiverDestroyed(*this);
}

//-----------------------------------------------------------------------

void Receiver::disconnectFromMessage(const char * const messageTypeName)
{
	disconnectFromMessage(MessageBase::makeMessageTypeFromString(messageTypeName));
}

//-----------------------------------------------------------------------

void Receiver::disconnectFromMessage(const unsigned long int messageType)
{
	MessageManager::getInstance().removeReceiver(*this, messageType);
}

//-----------------------------------------------------------------------

void Receiver::disconnectFromMessage(const MessageBase & source)
{
	disconnectFromMessage(source.getType());
}

//---------------------------------------------------------------------
/**
	@brief helper to clean up destroyed emitter objects

	If an Emitter is destroyed, but is not removed from it's target
	Receiver objects emitterTargets, the Receiver objects could
	attempt to invoke methods on an Emitter that no longer exists (e.g.
	during destruction, when the Reciever attempts to notify the
	Emitter that it is going away).

	@param target    The Emitter that is being destroyed

	@see Emitter::receiverDestroyed

	@author Justin Randall
*/
void Receiver::emitterDestroyed(Emitter & target)
{
	emitterTargets->c.erase (&target);
}

//----------------------------------------------------------------------
/**
	@brief stop receiving a specific message type from an emitter

	@param target      The Emitter that we want to stop listening for messagetype from
	@param messageType The messagetype we want to stop listening to on the target

	@see Emitter::connectToEmitter

	@author John Watson
*/
void Receiver::disconnectFromEmitter(const Emitter & target, const char * const messageType)
{
	target.removeReceiver (*this, messageType);
	if (!target.hasReceiver (*this))
		emitterTargets->c.erase (&target);
}

//---------------------------------------------------------------------

GlobalFunctionReceiver::GlobalFunctionReceiver(Function function)
: Receiver(), 
	m_function(function)
{
}

//---------------------------------------------------------------------

GlobalFunctionReceiver::~GlobalFunctionReceiver()
{
}

//---------------------------------------------------------------------

void GlobalFunctionReceiver::receiveMessage(const Emitter &source, const MessageBase &message)
{
	m_function(source, message);
}

//---------------------------------------------------------------------

}//namespace MessageDispatch
