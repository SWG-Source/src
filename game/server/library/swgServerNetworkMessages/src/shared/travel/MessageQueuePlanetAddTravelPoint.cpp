// ======================================================================
//
// MessageQueuePlanetAddTravelPoint.cpp
// asommers
// 
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueuePlanetAddTravelPoint.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE (MessageQueuePlanetAddTravelPoint, CM_planetAddTravelPoint);

// ======================================================================

MessageQueuePlanetAddTravelPoint::MessageQueuePlanetAddTravelPoint (const std::string& travelPointName, const Vector& position_w, const int cost, const bool interplanetary, const uint32 type) :
	MessageQueue::Data (),
	m_travelPointName (travelPointName),
	m_position_w (position_w),
	m_cost (cost),
	m_interplanetary (interplanetary),
	m_type (type)
{
}

// ----------------------------------------------------------------------

MessageQueuePlanetAddTravelPoint::~MessageQueuePlanetAddTravelPoint ()
{
}

// ----------------------------------------------------------------------

const std::string& MessageQueuePlanetAddTravelPoint::getTravelPointName () const
{
	return m_travelPointName;
}

// ----------------------------------------------------------------------

const Vector& MessageQueuePlanetAddTravelPoint::getPosition_w () const
{
	return m_position_w;
}

// ----------------------------------------------------------------------

int MessageQueuePlanetAddTravelPoint::getCost () const
{
	return m_cost;
}

// ----------------------------------------------------------------------

bool MessageQueuePlanetAddTravelPoint::getInterplanetary () const
{
	return m_interplanetary;
}

// ----------------------------------------------------------------------

uint32 MessageQueuePlanetAddTravelPoint::getType () const
{
	return m_type;
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueuePlanetAddTravelPointArchive::get (Archive::ReadIterator& source)
{
	std::string name;
	Archive::get (source, name);

	Vector position_w;
	Archive::get (source, position_w);

	int cost = 0;
	Archive::get (source, cost);

	bool interplanetary = false;
	Archive::get (source, interplanetary);

	uint32 type = 0;
	Archive::get (source, type);

	return new MessageQueuePlanetAddTravelPoint (name, position_w, cost, interplanetary, type);
}

//----------------------------------------------------------------------

void MessageQueuePlanetAddTravelPointArchive::put (const MessageQueue::Data* const source, Archive::ByteStream& target)
{
	const MessageQueuePlanetAddTravelPoint* const message = dynamic_cast<const MessageQueuePlanetAddTravelPoint*> (source);
	NOT_NULL (message);

	if (message)
	{
		Archive::put (target, message->getTravelPointName ());
		Archive::put (target, message->getPosition_w ());
		Archive::put (target, message->getCost ());
		Archive::put (target, message->getInterplanetary ());
		Archive::put (target, message->getType ());
	}
}

// ======================================================================
