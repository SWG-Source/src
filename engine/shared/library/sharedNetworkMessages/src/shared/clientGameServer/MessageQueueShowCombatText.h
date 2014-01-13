//======================================================================
//
// MessageQueueShowCombatText.h
// copyright (c) 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueShowCombatText_H
#define INCLUDED_MessageQueueShowCombatText_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "StringId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------
/**
 * A message to instruct a client object to place fly text over it.
 *
 * Fly text is the text that shows up over an object, like the damage
 * numbers over combatants, that moves upward and fades out over time.
 **/

class MessageQueueShowCombatText : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	enum Flags
	{
		F_private        = 0x0001,
	};

	MessageQueueShowCombatText (const NetworkId &defenderId, const NetworkId &attackerId, const StringId &outputTextId, float scale, int r, int g, int b, int flags);
	MessageQueueShowCombatText (const NetworkId &defenderId, const NetworkId &attackerId, const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags);

	const NetworkId & getDefenderId   () const;
	const NetworkId & getAttackerId   () const;
	const StringId &  getOutputTextId () const;
	const Unicode::String & getOutputTextOOB() const;
	float             getScale        () const;
	int               getRed          () const;
	int               getGreen        () const;
	int               getBlue         () const;
	int               getFlags        () const;

private:

	// Disabled.
	MessageQueueShowCombatText ();
	MessageQueueShowCombatText (const MessageQueueShowCombatText &);
	MessageQueueShowCombatText& operator=(const MessageQueueShowCombatText&);

private:

	NetworkId m_defenderId;
	NetworkId m_attackerId;
	StringId  m_outputTextId;
	Unicode::String m_outputTextOOB;
	float     m_scale;
	int       m_r;
	int       m_g;
	int       m_b;
	int       m_flags;
};

//======================================================================

inline const NetworkId &MessageQueueShowCombatText::getDefenderId() const
{
	return m_defenderId;
}

// ----------------------------------------------------------------------

inline const NetworkId &MessageQueueShowCombatText::getAttackerId() const
{
	return m_attackerId;
}

// ----------------------------------------------------------------------

inline const StringId &MessageQueueShowCombatText::getOutputTextId() const 
{ 
	return m_outputTextId;
}

// ----------------------------------------------------------------------

inline const Unicode::String &MessageQueueShowCombatText::getOutputTextOOB() const 
{ 
	return m_outputTextOOB;
}

//----------------------------------------------------------------------

inline float MessageQueueShowCombatText::getScale() const
{
	return m_scale;
}

//----------------------------------------------------------------------

inline int MessageQueueShowCombatText::getRed() const
{
	return m_r;
}

//----------------------------------------------------------------------

inline int MessageQueueShowCombatText::getGreen() const
{
	return m_g;
}

//----------------------------------------------------------------------

inline int MessageQueueShowCombatText::getBlue() const
{
	return m_b;
}

//----------------------------------------------------------------------

inline int MessageQueueShowCombatText::getFlags        () const
{
	return m_flags;
}

//======================================================================

#endif
