//======================================================================
//
// MessageQueueShipHit.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueShipHit_H
#define INCLUDED_MessageQueueShipHit_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

#include "sharedMath/Vector.h"

class MemoryBlockManager;

//----------------------------------------------------------------------

/**
* the 'type' field _must_ be one of ShipHitEffectsManager::HitType
*/ 

class MessageQueueShipHit: public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueueShipHit(Vector const & _up_w, Vector const & _hitLocation_o, int _type, float _integrity, float _previousIntegrity);
	MessageQueueShipHit();

private:
	MessageQueueShipHit (const MessageQueueShipHit & rhs);
	MessageQueueShipHit & operator= (const MessageQueueShipHit & rhs);

public:

	Vector up_w;
	Vector hitLocation_o;
	int type;
	float integrity;
	float previousIntegrity;
};

//======================================================================

#endif
