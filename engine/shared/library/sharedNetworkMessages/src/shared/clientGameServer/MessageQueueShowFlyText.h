//======================================================================
//
// MessageQueueShowFlyText.h
// copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueShowFlyText_H
#define INCLUDED_MessageQueueShowFlyText_H

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

class MessageQueueShowFlyText : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	enum Flags
	{
		F_private            = 0x0001,
		F_showInChatBox      = 0x0002,
		F_isDamageFromPlayer = 0x0004,
		F_isSnare            = 0x0008,
		F_isGlancingBlow     = 0x0010,
		F_isCriticalHit      = 0x0020,
		F_isLucky            = 0x0040,
		F_isDot              = 0x0080,
		F_isBleed            = 0x0100,
		F_isHeal             = 0x0200,
		F_isFreeshot         = 0x0400
	};

	MessageQueueShowFlyText (const NetworkId &emitterId, const StringId &outputTextId, float scale, int r, int g, int b, int flags);
	MessageQueueShowFlyText (const NetworkId &emitterId, const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags);

	const NetworkId & getEmitterId    () const;
	const StringId &  getOutputTextId () const;
	const Unicode::String & getOutputTextOOB () const;
	float             getScale        () const;
	int               getRed          () const;
	int               getGreen        () const;
	int               getBlue         () const;
	int               getFlags        () const;

private:

	// Disabled.
	MessageQueueShowFlyText ();
	MessageQueueShowFlyText (const MessageQueueShowFlyText &);
	MessageQueueShowFlyText& operator=(const MessageQueueShowFlyText&);

private:

	NetworkId m_emitterId;
	StringId  m_outputTextId;
	Unicode::String m_outputTextOOB;
	float     m_scale;
	int       m_r;
	int       m_g;
	int       m_b;
	int       m_flags;
};

//======================================================================

inline const NetworkId &MessageQueueShowFlyText::getEmitterId() const
{
	return m_emitterId;
}

// ----------------------------------------------------------------------

inline const StringId &MessageQueueShowFlyText::getOutputTextId() const 
{ 
	return m_outputTextId;
}

// ----------------------------------------------------------------------

inline const Unicode::String &MessageQueueShowFlyText::getOutputTextOOB() const 
{ 
	return m_outputTextOOB;
}

//----------------------------------------------------------------------

inline float MessageQueueShowFlyText::getScale() const
{
	return m_scale;
}

//----------------------------------------------------------------------

inline int MessageQueueShowFlyText::getRed() const
{
	return m_r;
}

//----------------------------------------------------------------------

inline int MessageQueueShowFlyText::getGreen() const
{
	return m_g;
}

//----------------------------------------------------------------------

inline int MessageQueueShowFlyText::getBlue() const
{
	return m_b;
}

//----------------------------------------------------------------------

inline int MessageQueueShowFlyText::getFlags        () const
{
	return m_flags;
}

//======================================================================

#endif
