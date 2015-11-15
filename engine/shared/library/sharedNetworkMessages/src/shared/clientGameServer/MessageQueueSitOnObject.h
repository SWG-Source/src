//======================================================================
//
// MessageQueueSitOnObject.h
// copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#ifndef INCLUDED_MessageQueueSitOnObject_H
#define INCLUDED_MessageQueueSitOnObject_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------
/**
 * A message to instruct a CreatureObject to sit on another Object.
 *
 * This function is intended for use with the Postures::Sitting posture
 * (e.g. sitting on a chair or couch).  It is not meant to handle the action 
 * of mounting an Object.
 */

class MessageQueueSitOnObject : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	static float const cs_maximumChairRange;
	static float const cs_maximumChairRangeSquared;

public:
	
	explicit MessageQueueSitOnObject(NetworkId const &chairCellId, Vector const &chairPosition_p);

	NetworkId const &getChairCellId() const;
	Vector const    &getChairPosition_p() const;

private:

	// Disabled.
	MessageQueueSitOnObject();
	MessageQueueSitOnObject& operator=(const MessageQueueSitOnObject&);

private:

	NetworkId  m_chairCellId;
	Vector     m_chairPosition_p;

};

//======================================================================

inline NetworkId const &MessageQueueSitOnObject::getChairCellId() const
{
	return m_chairCellId;
}

// ----------------------------------------------------------------------

inline Vector const &MessageQueueSitOnObject::getChairPosition_p() const 
{ 
	return m_chairPosition_p;
}

//======================================================================

#endif
