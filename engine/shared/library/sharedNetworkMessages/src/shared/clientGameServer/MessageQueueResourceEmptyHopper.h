//======================================================================
//
// MessageQueueResourceEmptyHopper.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueResourceEmptyHopper_H
#define INCLUDED_MessageQueueResourceEmptyHopper_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//======================================================================

/**
* The proper response to a MessageQueueResourceEmptyHopper is a MessageQueueGenericResponse
*/

class MessageQueueResourceEmptyHopper : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	friend class MessageQueueResourceEmptyHopperArchive;

	/**
	*
	* @param playerId
	* @param harvesterId
	* @param resourceId 
	* @param amount a negative amount indicates that you are referring to _ALL_ of the specified resource in the hopper
	* @param discard simply delete the resources, otherwise attempt to give it to player
	*/

	MessageQueueResourceEmptyHopper (const NetworkId & playerId, const NetworkId & harvesterId, const NetworkId & resourceId, int amount, bool discard, uint8 sequenceId);
	~MessageQueueResourceEmptyHopper ();

	uint8               getSequenceId  () const;
	const NetworkId &   getPlayerId    () const;
	const NetworkId &   getHarvesterId () const;
	const NetworkId &   getResourceId  () const;
	const int           getAmount      () const;
	const bool          getDiscard     () const;

private:
	MessageQueueResourceEmptyHopper ();
	MessageQueueResourceEmptyHopper (const MessageQueueResourceEmptyHopper &);
	MessageQueueResourceEmptyHopper & operator= (const MessageQueueResourceEmptyHopper &);

	NetworkId   m_playerId;
	NetworkId   m_harvesterId;
	NetworkId   m_resourceId;
	int         m_amount;
	bool        m_discard;

	uint8       m_sequenceId;
};

//----------------------------------------------------------------------

inline uint8               MessageQueueResourceEmptyHopper::getSequenceId  () const
{
	return m_sequenceId;
}

//----------------------------------------------------------------------

inline const NetworkId &   MessageQueueResourceEmptyHopper::getPlayerId    () const
{
	return m_playerId;
}

//----------------------------------------------------------------------

inline const NetworkId &   MessageQueueResourceEmptyHopper::getHarvesterId () const
{
	return m_harvesterId;
}

//----------------------------------------------------------------------

inline const NetworkId &   MessageQueueResourceEmptyHopper::getResourceId  () const
{
	return m_resourceId;
}

//----------------------------------------------------------------------

inline const int           MessageQueueResourceEmptyHopper::getAmount      () const
{
	return m_amount;
}

//----------------------------------------------------------------------

inline const bool          MessageQueueResourceEmptyHopper::getDiscard     () const
{
	return m_discard;
}

//======================================================================

#endif
