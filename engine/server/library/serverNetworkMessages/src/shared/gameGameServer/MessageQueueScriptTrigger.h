// MessageQueueScriptTrigger.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueScriptTrigger_H
#define	_INCLUDED_MessageQueueScriptTrigger_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
namespace Archive {
	class ByteStream;
}

//-----------------------------------------------------------------------

class MessageQueueScriptTrigger : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueScriptTrigger(int trigId, const Archive::ByteStream & scriptParamData);
	~MessageQueueScriptTrigger();

	const Archive::ByteStream &  getScriptParamData  () const;
	const int                    getTriggerId        () const;

private:
	MessageQueueScriptTrigger & operator = (const MessageQueueScriptTrigger & rhs);
	MessageQueueScriptTrigger(const MessageQueueScriptTrigger & source);

	Archive::ByteStream  scriptParamData;
	int                  triggerId;
};

//-----------------------------------------------------------------------

inline const Archive::ByteStream & MessageQueueScriptTrigger::getScriptParamData () const
{
	return scriptParamData;
}

//-----------------------------------------------------------------------

inline const int MessageQueueScriptTrigger::getTriggerId () const
{
	return triggerId;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueScriptTrigger_H
