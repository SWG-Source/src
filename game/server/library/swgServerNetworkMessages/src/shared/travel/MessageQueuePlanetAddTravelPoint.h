// ======================================================================
//
// MessageQueuePlanetAddTravelPoint.h
// asommers
// 
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueuePlanetAddTravelPoint_H
#define INCLUDED_MessageQueuePlanetAddTravelPoint_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

// ======================================================================

class MessageQueuePlanetAddTravelPoint : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	MessageQueuePlanetAddTravelPoint (const std::string& travelPointName, const Vector& position_w, int cost, bool interplanetary, uint32 type);
	virtual ~MessageQueuePlanetAddTravelPoint ();

	const std::string& getTravelPointName () const;
	const Vector&      getPosition_w () const;
	int                getCost () const;
	bool               getInterplanetary () const;
	uint32             getType () const;
	
private:

	MessageQueuePlanetAddTravelPoint ();
	MessageQueuePlanetAddTravelPoint (const MessageQueuePlanetAddTravelPoint&);
	MessageQueuePlanetAddTravelPoint& operator= (const MessageQueuePlanetAddTravelPoint&);

private:

	const std::string m_travelPointName;
	const Vector      m_position_w;
	const int         m_cost;
	const bool        m_interplanetary;
	const uint32      m_type;
};

// ======================================================================

class MessageQueuePlanetAddTravelPointArchive
{
public:

	static MessageQueue::Data* get (Archive::ReadIterator& source);
	static void                put (const MessageQueue::Data* source, Archive::ByteStream& target);
};

// ======================================================================

#endif
