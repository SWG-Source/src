// ControllerMessageFactory.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	INCLUDED_ControllerMessageFactory_H
#define	INCLUDED_ControllerMessageFactory_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

//-----------------------------------------------------------------------
//@todo This is causing a dependency on the game.  We need to break this up.
class ControllerMessageFactory
{
public:

	static void                install();
	static void                remove();

	typedef void (*ControllerMessagePackFunction)(const MessageQueue::Data* data, Archive::ByteStream & target);
	typedef MessageQueue::Data* (*ControllerMessageUnpackFunction)(Archive::ReadIterator & source);

	static void registerControllerMessageHandler (int32 message, ControllerMessagePackFunction, ControllerMessageUnpackFunction, bool allowFromClient = false);

	static void                pack                                     (const int32 message, const MessageQueue::Data* data, Archive::ByteStream & target);
	static MessageQueue::Data* unpack                                   (const int32 message, Archive::ReadIterator & source);
	static bool                allowFromClient                          (const int32 message);

private:

	ControllerMessageFactory ();
	ControllerMessageFactory (const ControllerMessageFactory & source);
	ControllerMessageFactory& operator= (const ControllerMessageFactory & rhs);
	~ControllerMessageFactory ();
};

//-----------------------------------------------------------------------

#endif	// _ControllerMessageFactory_H

