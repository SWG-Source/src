//========================================================================
//
// MessageQueueResourceFinishHarvest.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueResourceFinishHarvest_H
#define INCLUDED_MessageQueueResourceFinishHarvest_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"

/**
 * Sent by:  Resource Pool
 * Sent to:  Harvester
 * Action:  Last step of the harvest action.  The resource pool reports
 * the amount collected and the new tick count.
 */

class MessageQueueResourceFinishHarvest : public MessageQueue::Data
{
public:
	MessageQueueResourceFinishHarvest(float amountCollected, float tickCount, const NetworkId &resourceId);
	virtual ~MessageQueueResourceFinishHarvest();
	
	MessageQueueResourceFinishHarvest&	operator=	(const MessageQueueResourceFinishHarvest & source);
	MessageQueueResourceFinishHarvest(const MessageQueueResourceFinishHarvest & source);

	float getAmountCollected() const;
	float getTickCount() const;
	const NetworkId &getResourceId() const;
	
private:
	float m_amountCollected;
	float m_tickCount;
	NetworkId m_resourceId;
};

// ======================================================================

inline float MessageQueueResourceFinishHarvest::getTickCount() const
{
	return m_tickCount;
}

// ----------------------------------------------------------------------

inline float MessageQueueResourceFinishHarvest::getAmountCollected() const
{
	return m_amountCollected;
}

// ----------------------------------------------------------------------

inline const NetworkId &MessageQueueResourceFinishHarvest::getResourceId() const
{
	return m_resourceId;
}

// ======================================================================

#endif
