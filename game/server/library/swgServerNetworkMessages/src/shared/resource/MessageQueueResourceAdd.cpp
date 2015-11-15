// ======================================================================
//
// MessageQueueResourceAdd.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "MessageQueueResourceAdd.h"

#include <string>

// ======================================================================

MessageQueueResourceAdd::MessageQueueResourceAdd(const NetworkId &resourceId, int32 amount) :
		m_resourceId(resourceId),
		m_amount(amount)
{
}

// ======================================================================

MessageQueueResourceAdd::~MessageQueueResourceAdd()
{
}

// ======================================================================
