// ======================================================================
//
// MessageQueueAlterAttribute.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueAlterAttribute.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueAlterAttribute, CM_alterAttribute);

//===================================================================


// ----------------------------------------------------------------------

MessageQueueAlterAttribute::MessageQueueAlterAttribute(int attrib, int delta, 
	bool checkIncapacitation, const NetworkId & source) :
	m_attrib(attrib),
	m_delta(delta),
	m_checkIncapacitation(checkIncapacitation),
	m_source(source)
{
}	

// ----------------------------------------------------------------------

MessageQueueAlterAttribute::~MessageQueueAlterAttribute()
{
}

void MessageQueueAlterAttribute::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueAlterAttribute* const msg = safe_cast<const MessageQueueAlterAttribute*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_attrib);
		Archive::put(target, msg->m_delta);
		Archive::put(target, msg->m_checkIncapacitation);
		Archive::put(target, msg->m_source);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueAlterAttribute::unpack(Archive::ReadIterator & source)
{
	int attrib;
	int delta;
	bool checkIncapacitation;
	NetworkId attacker;

	Archive::get(source, attrib);
	Archive::get(source, delta);
	Archive::get(source, checkIncapacitation);
	Archive::get(source, attacker);
	
	return new MessageQueueAlterAttribute(attrib, delta, checkIncapacitation, attacker);
}


// ======================================================================
