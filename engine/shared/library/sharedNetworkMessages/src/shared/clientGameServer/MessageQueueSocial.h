//======================================================================
//
// MessageQueueSocial.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueSocial_H
#define INCLUDED_MessageQueueSocial_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A social message is issued on an server object's MessageQueue if the object
* observes 'source' performing social towards 'target'.
*
* A social message is issued on an client object's MessageQueue if it
* wants to perform a social.
*
*/

//----------------------------------------------------------------------

class MessageQueueSocial : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	MessageQueueSocial (const NetworkId & sourceId, const NetworkId & targetId, uint32 socialType, bool animationOk, bool textOk);
	MessageQueueSocial (const MessageQueueSocial&);

	const NetworkId &       getSourceId   () const;
	const NetworkId &       getTargetId   () const;
	const uint32            getSocialType () const;
	const bool              isAnimationOk () const;
	const bool              isTextOk      () const;

private:

	MessageQueueSocial ();
	MessageQueueSocial& operator= (const MessageQueueSocial&);

public:

	enum Flags
	{
		F_animationOk   = 0x0001,
		F_textOk        = 0x0002
	};

	NetworkId  m_sourceId;
	NetworkId  m_targetId;
	uint32     m_socialType;
	uint8      m_flags;
};

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueSocial::getSourceId   () const { return m_sourceId; }
inline const NetworkId & MessageQueueSocial::getTargetId   () const { return m_targetId; }
inline const uint32      MessageQueueSocial::getSocialType () const { return m_socialType; }
inline const bool        MessageQueueSocial::isAnimationOk () const { return (m_flags & F_animationOk) != 0; }
inline const bool        MessageQueueSocial::isTextOk      () const { return (m_flags & F_textOk)      != 0; }

//======================================================================

#endif
