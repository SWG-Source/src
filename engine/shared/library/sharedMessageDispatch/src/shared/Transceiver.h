// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#ifndef	_Transceiver_H
#define	_Transceiver_H

/**
	Transceiver.h

	Message dispatch to non-static object member functions 


                    <----- Messages from Transceivers
                             deliver to OBJECT::foo(Type)
							 which is a non-static member 
							 function.
                    <-----------------------
	+----------------------+          +-------------------+
	| OBJECT               |      +---+ Transceiver<Type> |
	+-+--------------------+      |   +-------------------+
	| |Callback m_callback |      | 
	| +-+------------------+      |   +-------------------+
	|   |OwnedTransceiver  |------+---+ Transceiver<Type> |
	|   +------------------+      |   +-------------------+
	|   |ptr to member func|      |
	|   +------------------+      |   +-------------------+
	+                      |      +---+ Transceiver<Type> +
	| +--------------------+          +-------------------+
	+-|void foo(Type)      |
	| +--------------------+
	|                      |
	+----------------------+  

	Transceivers may be used as anonymous "broadcast" sources
	delivering messages to anyone listing for a particular *type*
	of message, or they may have direct relationships with other
	transceivers. The relationship is defined by the receiving
	object, not the transceiver. The receiver will connect() to
	either another transceiver, or to a type of message. It specifies
	the address of a member function that is capable of receiving
	the type of message emitted from another transceiver.

	When the emitting transceiver dispatches a message, it looks
	through a vector of receivers for direct message dispatch, then
	delivers messages to each receiver in the list. It also maintains
	a list of pending removes, and skips transceivers (really 
	OwnedTransceiver objects) that have been deleted to prevent
	dispatch to a destroyed object.

	After direct dispatches are delievered, the transceiver finds
	a "gobal" dispatch list that contains receivers that are connected
	to the *type* of message this transceiver emits, rahter than this
	transceiver specifically. The message is delivered to all recepients
	in that global list. Like the local dispatch, the global dispatch also
	checks for destroyed objects and skips delivery if they are no
	longer valid.

	Clients of dispatch code use to interfaces: Callback objects, which
	are responsible for connecting object member functions to transceivers,
	and Transceiver objects themselves. Client code may either implement
	directed dispatch from object to object using a has-a relationship. The
	emitting object will have a Transceiver, the receiving object will
	have a Callback. It is the responsibility of the client to ensure
	that their Transceiver or Callback objects are destroyed when their
	owning objects are destroyed. 

	Client code may also emit messages by simply declaring a Transceiver
	on the stack and invoking emitMessage() with the correct parameter type
	for the message. Any objects that connect() to a message type rather
	than a specific transceiver will receive the message.

	Usage:

	// anonymous dispatch example
	class MyClass
	{
	public:
		MyClass()
		{
			m_callback = new Callback;
			// connect to any transceiver emitting a bool
			m_callback->connect(*this, &MyClass::foo);
		}
		~MyClass()
		{
			// ensure transceivers don't deliver to this 
			// object anymore
			delete m_callback;
		}

	protected:
		void foo(bool);
	private:
		Callback * m_callback;
	};

	// code somewhere else (could be anywhere)
	void bar()
	{
		Transceiver<bool> t;
		bool msg = true;
		t.emitMessage(msg);
	}

	// directed dispatch example
	class MyEmitter
	{
	public:
		//...

		Transceiver<bool> & getEmitter()
		{	
			return m_emitter;
		}

		void bar()
		{
			bool msg = true;
			m_emitter.emitMessage(msg);
		}

	private:
		Transceiver<bool> m_emitter;
	};

	class MyReceiver
	{
	public:
		explicit MyReceiver(MyEmitter & source)
		{
			m_callback = new Callback;
			m_callback->connect(source, *this, &MyReceiver::foo);
		}
		~MyReceiver()
		{
			delete m_callback;
		}
	protected:
		void foo(bool)
		{
			// I received a bool message from one of 
			// the transceivers I am connected too!
		}
	};

	// anywhere else in the application
	void doStuff()
	{
		MyEmitter emitter;
		MyReceiver receiver(emitter);

		// receiver.foo(true) is called by way of emitter.bar()
		emitter.bar();
	}
*/ 
//-----------------------------------------------------------------------

#include <algorithm>
#include <set>
#include <typeinfo>
#include <vector>

#if _MSC_VER < 1300
using std::type_info;
#endif


//---------------------------------------------------------------------

namespace MessageDispatch {

class TransceiverBase 
{
public:
	TransceiverBase();
	virtual ~TransceiverBase();


protected:
	struct GlobalReceiverInfo
	{
		GlobalReceiverInfo();
		std::vector<TransceiverBase *>  receivers;
		std::vector<TransceiverBase *>  pendingAdds;
		std::vector<TransceiverBase *>  pendingRemoves;

		bool                            locked;
	};

protected:
	static GlobalReceiverInfo & getGlobalReceiverInfo(const type_info & typeId);

protected:
	mutable bool locked;
};

template<typename MessageType, typename IdentifierType = void *>
class Transceiver : public TransceiverBase
{
public:
	Transceiver();
	virtual ~Transceiver();
	void emitMessage(MessageType source) const;
protected:
	friend class Callback;
	void listenForAny();
	//void listenTo(Transceiver<MessageType, IdentifierType> & source);
	virtual void receiveMessage(MessageType) {};

private:
	Transceiver(const Transceiver & source);
	Transceiver & operator = (const Transceiver &  rhs);

	void addReceiver(Transceiver * target);
	void removeReceiver(Transceiver * target);

private:
	std::vector<Transceiver<MessageType, IdentifierType> *> localReceivers;
	std::set<Transceiver<MessageType, IdentifierType> *> listenSet;

	// bookkeeping containers and flags to permit safe usage during
	// message emission
	mutable std::vector<Transceiver<MessageType, IdentifierType> *> pendingAdds;
	mutable std::vector<Transceiver<MessageType, IdentifierType> *> pendingRemoves;
};

//---------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline Transceiver<MessageType, IdentifierType>::Transceiver() :
TransceiverBase(),
localReceivers(),
listenSet(),
pendingAdds(),
pendingRemoves()
{
	// ensure the global receiver for this type of transceiver
	// is fully constructed before this object (which may be the
	// first transceiver) is fully constructed. The global
	// receiver info is a static with linkage in Transceiver.cpp,
	// and should be destroyed only after ALL transceivers of this
	// type have been destroyed.
	getGlobalReceiverInfo(typeid(this));
}

//---------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline Transceiver<MessageType, IdentifierType>::~Transceiver()
{
	typename std::vector<Transceiver<MessageType, IdentifierType> *>::iterator i;
	typename std::set<Transceiver<MessageType, IdentifierType> *>::iterator f;

	// advise anyone that this transceiver is connected to that
	// they should NOT call removeReceiver in their destructor
	// because THIS transceiver is being destroyed
	for(i = localReceivers.begin(); i != localReceivers.end(); ++i)
	{
		// find this receiver on the other transceiver's
		// listen set
		f = (*i)->listenSet.find(this);

		// if it's found, remove this receiver from the listenSet
		if(f != (*i)->listenSet.end())
			(*i)->listenSet.erase(f);
	}

	// for every transceiver that this object receives messages
	// from, remove thiis object from their distribution
	// list, so that they will not attempt to deliver 
	// messages to this object after it has been destroyed
	for(f = listenSet.begin(); f != listenSet.end(); ++f)
	{
		(*f)->removeReceiver(this);
	}
	

	// remove this transceiver from the global receiver list
	// to prevent anonymous distribution to this delieted
	// transceiver
	GlobalReceiverInfo & info = getGlobalReceiverInfo(typeid(this));
	std::vector<TransceiverBase *>::iterator g;
	if(info.locked == false)
	{
		g = std::find(info.receivers.begin(), info.receivers.end(), this);
		if(g != info.receivers.end())
			info.receivers.erase(g);
	}
	else
	{
		// the global transceiver list is busy dispatching
		// a message that this transceiver may receive, put
		// it on the global pending remove list, which is
		// checked during message dispatch
		info.pendingRemoves.push_back(this);
	}
}

//-----------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline void Transceiver<MessageType, IdentifierType>::addReceiver(Transceiver<MessageType, IdentifierType> * target)
{
	// do not add a transceiver to the receiver list if this transceiver
	// is busy dispatching a message. 
	if(locked)
	{
		pendingAdds.push_back(target);
	}
	else
	{
		typename std::vector<Transceiver<MessageType, IdentifierType> *>::const_iterator f = std::find(localReceivers.begin(), localReceivers.end(), target);
		if(f == localReceivers.end())
		{
			localReceivers.push_back(target);
			target->listenSet.insert(this);
		}
	}
}

//---------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline void Transceiver<MessageType, IdentifierType>::emitMessage(MessageType message) const
{
	
	//-----------------------------------------------------------------------
	// set our status to locked so that adds to this receiver do not touch
	// the localReceiverSet and put it in an undefined state.
	// See Transceiver::addReceiver for information on the locked flag
	locked = true;

	// iterate through direct connections to this Transceiver
	typename std::vector<Transceiver<MessageType, IdentifierType> *>::const_iterator i;

	// which dispatch loop will we run?
		// fast dispatch
	for(i = localReceivers.begin(); i != localReceivers.end(); ++i)
	{
		// quick and easy check.
		// if no removes have been posted as a result of the
		// last dispatch, simply deliver the message
		if(pendingRemoves.empty())
		{
			(*i)->receiveMessage(message);
		}
		else
		{
			// it might not be safe, because a transceiver requested a removal. This
			// can happen when the owner object of the transceiver has
			// deleted itself. Just skip this target and clean up the
			// removal at the end of emitMessage.
			if(std::find(pendingRemoves.begin(), pendingRemoves.end(), (*i)) != pendingRemoves.end())
				continue;

			(*i)->receiveMessage(message);
		}
	}
	locked = false;
	//-----------------------------------------------------------------------

	
	//-----------------------------------------------------------------------
	// lock the global receiver set to prevent
	// unsafe add/remove calls to it while dispatching this message.
	GlobalReceiverInfo & info = getGlobalReceiverInfo(typeid(const_cast<Transceiver<MessageType, IdentifierType> *>(this)));
	info.locked = true;
	std::vector<TransceiverBase *>::iterator g;
	for(g = info.receivers.begin(); g != info.receivers.end(); ++g)
	{
		// quick and easy check.
		// if no removes have been posted as a result of the
		// last dispatch, simply deliver the message
		if(pendingRemoves.empty())
		{
			Transceiver<MessageType, IdentifierType> * t = static_cast<Transceiver<MessageType, IdentifierType> *>((*g));
			t->receiveMessage(message);
		}
		else
		{
			// it might not be safe, because a transceiver requested a removal. This
			// can happen when the owner object of the transceiver has
			// deleted itself. Just skip this target and clean up the
			// removal at the end of emitMessage.
			if(std::find(info.pendingRemoves.begin(), info.pendingRemoves.end(), (*g)) != info.pendingRemoves.end())
				continue;

			Transceiver<MessageType, IdentifierType> * t = static_cast<Transceiver<MessageType, IdentifierType> *>((*g));
			t->receiveMessage(message);
		}
	}
	info.locked = false;
	//-----------------------------------------------------------------------

	// process local connections to this transceiver
	if(! pendingAdds.empty())
	{
		for(i = pendingAdds.begin(); i != pendingAdds.end(); ++i)
		{
			// place the pending transceive ron the receiver list
			const_cast<Transceiver<MessageType, IdentifierType> *>(this)->addReceiver(*i);
		}
		pendingAdds.clear();
	}
	
	// a transceiver directly connected to this object was destroyed or
	// disconnected during the message dispatch. Remove it from the
	// list of receivers to ensure messages aren't delivered to the target
	// and that it will not receive a notification during the destruction
	// of THIS transceiver
	if(! pendingRemoves.empty())
	{
		typename std::vector<Transceiver<MessageType, IdentifierType> *>::const_iterator a;;
		for(a = pendingRemoves.begin(); a != pendingRemoves.end(); ++a)
		{
			const_cast<Transceiver<MessageType, IdentifierType> *>(this)->removeReceiver(*a);
		}
		pendingRemoves.clear();
	}

	// perform the same add/remove logic for the global receiver info
	if(! info.pendingAdds.empty())
	{
		std::vector<TransceiverBase *>::const_iterator addIter;
		for(addIter = info.pendingAdds.begin(); addIter != info.pendingAdds.end(); ++addIter)
		{
			Transceiver<MessageType, IdentifierType> * t = static_cast<Transceiver<MessageType, IdentifierType> *>((*addIter));
			t->listenForAny();
		}
		info.pendingAdds.clear();
	}

	if(! info.pendingRemoves.empty())
	{
		std::vector<TransceiverBase *>::const_iterator removeIter;
		for(removeIter = info.pendingRemoves.begin(); removeIter != info.pendingRemoves.end(); ++removeIter)
		{
			std::vector<TransceiverBase *>::iterator r = std::find(info.receivers.begin(), info.receivers.end(), *removeIter);
			if(r != info.receivers.end())
				info.receivers.erase(r);
		}
		info.pendingRemoves.clear();
	}
}

//---------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline void Transceiver<MessageType, IdentifierType>::listenForAny()
{
	GlobalReceiverInfo & info = getGlobalReceiverInfo(typeid(this));

	// if the global receiver info is not locked, then it is not
	// busy dispatching an anonymous message and it is safe to
	// add this transceiver directly to the list of listeners
	if(! info.locked)
	{
		std::vector<TransceiverBase *>::const_iterator f = std::find(info.receivers.begin(), info.receivers.end(), this);
		if(f == info.receivers.end())
			info.receivers.push_back(this);
	}
	else
	{
		// another transceiver of this type is performing
		// operations on the global receiver set. Adds will
		// be processed by that transceiver when it is done
		// working with the global receiver list
		info.pendingAdds.push_back(this);
	}
}

//-----------------------------------------------------------------------

template<typename MessageType, typename IdentifierType>
inline void Transceiver<MessageType, IdentifierType>::removeReceiver(Transceiver<MessageType, IdentifierType> * target)
{
	if(locked)
	{
		// this transceiver is busy dispatching messages. Do not invalidate 
		// the receiver list by erasing a member while this object is 
		// iterating through it to dispatch. The remove list, if it is
		// not empty, will be checked to see if the target transceiver
		// is listed, and messages will not be dispatched to it.
		pendingRemoves.push_back(target);
	}
	else
	{
		// it is safe to directly remove the target transceiver from the 
		// receiver list.
		typename std::vector<Transceiver<MessageType, IdentifierType> *>::iterator f = std::find(localReceivers.begin(), localReceivers.end(), target);
		if(f != localReceivers.end())
			localReceivers.erase(f);
	}
}

//---------------------------------------------------------------------

/**
	An OwnedTransceiver is a helper/interface class that links end-point
	objects, Callback objects and transceivers. The abstraction is 
	present to make message dispatch safe when emitting messages to objects
	that may be deleted. Callbacks and OwnedTransceivers notify other
	transceivers if an object is being destroyed. Objects that receive
	messages have Callback members, and when those callback members
	are destroyed, they clean up transceiver receive lists to prevent
	operations on the deleted object.
*/
template<typename MessageType, typename ObjectType, typename IdentifierType = void *>
class OwnedTransceiver : public Transceiver<MessageType, IdentifierType>
{
public:
	OwnedTransceiver(Transceiver<MessageType, IdentifierType> * source, ObjectType & o);
	~OwnedTransceiver();

	// this where all messages are routed before finally being
	// delivered to the object member function specified in the
	// callback. receiveMessage overrides the base Transceiver method.
	virtual void receiveMessage(MessageType msg)
	{
		(owner.*callback)(msg);
	};

	void setCallback(void (ObjectType::*cb)(MessageType) )
	{
		callback = cb;
	};

private:
	friend class Callback;
	OwnedTransceiver<MessageType, ObjectType, IdentifierType> & operator = (const OwnedTransceiver<MessageType, ObjectType, IdentifierType> &);
	OwnedTransceiver(const OwnedTransceiver<MessageType, ObjectType, IdentifierType> &);
	Transceiver<MessageType, IdentifierType> * source;
	ObjectType & owner;
	void (ObjectType::*callback)(MessageType);
};

//---------------------------------------------------------------------

template<typename MessageType, typename ObjectType, typename IdentifierType>
inline OwnedTransceiver<MessageType, ObjectType, IdentifierType>::OwnedTransceiver(Transceiver<MessageType, IdentifierType> * s, ObjectType & o) :
source(s),
owner(o),
callback(0)
{
}

//-----------------------------------------------------------------------

template<typename MessageType, typename ObjectType, typename IdentifierType>
inline OwnedTransceiver<MessageType, ObjectType, IdentifierType>::~OwnedTransceiver()
{
}

//-----------------------------------------------------------------------

class Callback
{
public:
	Callback();
	virtual ~Callback();

	template<typename MessageType, typename ObjectType, typename IdentifierType>
		void connect(
		             Transceiver<MessageType, IdentifierType> & source, 
		             ObjectType & object,
		             void (ObjectType::*callback)(MessageType)
		            )
	{
		OwnedTransceiver<MessageType, ObjectType, IdentifierType> * target = new OwnedTransceiver<MessageType, ObjectType, IdentifierType>(&source, object);
		receivers.push_back(target);
		
		target->setCallback(callback);
		source.addReceiver(target);
		//target->listenTo(source);
	}

	template<typename MessageType, typename ObjectType>
		inline void connect(ObjectType & object, void (ObjectType::*callback)(MessageType))
	{
		OwnedTransceiver<MessageType, ObjectType> * target = new OwnedTransceiver<MessageType, ObjectType>(0, object);
		receivers.push_back(target);
		
		target->setCallback(callback);
		target->listenForAny();
	}

	template<typename MessageType, typename ObjectType, typename IdentifierType>
		inline void connect(ObjectType & object, void (ObjectType::*callback)(MessageType), IdentifierType *)
	{
		OwnedTransceiver<MessageType, ObjectType, IdentifierType> * target = new OwnedTransceiver<MessageType, ObjectType, IdentifierType>(0, object);
		receivers.push_back(target);
		
		target->setCallback(callback);
		target->listenForAny();
	}

	template<typename MessageType, typename ObjectType, typename IdentifierType>
		void disconnect(
		                Transceiver<MessageType, IdentifierType> & source,
						ObjectType & object,
						void (ObjectType::*callback)(MessageType)
		               )
	{
		removeTarget(&source, object, callback);
	}

	template<typename MessageType, typename ObjectType, typename IdentifierType>
		void disconnectByType(
		                Transceiver<MessageType, IdentifierType> & source,
						ObjectType & object,
						void (ObjectType::*callback)(MessageType),
	                    IdentifierType *
		               )
	{
		removeTarget(&source, object, callback);
	}

	template<typename MessageType, typename ObjectType>
		void disconnect(ObjectType & object, void (ObjectType::*callback)(MessageType) )
	{
		removeTarget(static_cast<Transceiver<MessageType> *>(0), object, callback);
	}

	template<typename MessageType, typename ObjectType, typename IdentifierType>
		void disconnect(ObjectType & object, void (ObjectType::*callback)(MessageType), IdentifierType *)
	{
		removeTarget(static_cast<Transceiver<MessageType, IdentifierType> *>(0), object, callback);
	}

private:
	template<typename MessageType, typename ObjectType, typename IdentifierType>
		void removeTarget(
		                  Transceiver<MessageType, IdentifierType> * source,
		                  ObjectType & object,
		                  void (ObjectType::*callback)(MessageType)
		                 )
	{
		std::vector<TransceiverBase *>::iterator i;
		OwnedTransceiver<MessageType, ObjectType, IdentifierType> * target;
		
		for(i = receivers.begin(); i != receivers.end(); ++i)
		{
			target = dynamic_cast<OwnedTransceiver<MessageType, ObjectType, IdentifierType> *>((*i));
			if(target)
			{
				if(&target->owner == &object && target->callback == callback && target->source == source)
				{
					delete target;
					receivers.erase(i);
					break;
				}
			}
		}
	}

	std::vector<TransceiverBase *> receivers;
};
	
template<typename MessageType>
void emitMessage(MessageType m)
{
	static Transceiver<MessageType> t;
	t.emitMessage(m);
}

//-----------------------------------------------------------------------
}//namespace MessageDispatch

#endif	// _Transceiver_H
