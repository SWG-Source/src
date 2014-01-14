// ======================================================================
//
// Emitter.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_Emitter_H
#define	INCLUDED_Emitter_H

// ======================================================================

namespace MessageDispatch
{

	class MessageBase;
	class Receiver;

	/**
		@brief Class that can emit messages

		Emitter objects can transmit Message objects, which are dispatched
		to receivers either directly, or via the MessageManager.

		@see Message
		@see MessageBase
		@see Receiver
		@see MessageManager

		@author Justin Randall
	*/
	class Emitter 
	{
	public:
								 Emitter();
		virtual      ~Emitter();
		void         addReceiver(Receiver & target, const char * const messageTypeName) const;
		void         addReceiver(Receiver & target, const MessageBase & source) const;
		void         addReceiver(Receiver & target, const unsigned long int messageType) const;
		void         emitMessage(const MessageBase & message) const;

		const bool   hasReceiver(const Receiver & target, const unsigned long int messageType) const;
		const bool   hasReceiver(const Receiver & target) const;

		void         receiverDestroyed(const Receiver & target) const;
		void         removeReceiver(const Receiver & target, const char * const messageTypeName) const;

	private:

		struct ReceiverList;
		ReceiverList * receiverList;
	};


}// namespace MessageDispatch

// ======================================================================

#endif

