// ======================================================================
//
// MessageQueueResourceFinishHarvest.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "MessageQueueResourceFinishHarvest.h"

// ======================================================================

MessageQueueResourceFinishHarvest::MessageQueueResourceFinishHarvest(float amountCollected, float tickCount, const NetworkId &resourceId) :
		m_amountCollected(amountCollected),
		m_tickCount(tickCount),
		m_resourceId(resourceId)
{
}

// ----------------------------------------------------------------------

MessageQueueResourceFinishHarvest::~MessageQueueResourceFinishHarvest()
{
}

// ======================================================================
