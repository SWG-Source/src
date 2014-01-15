// ======================================================================
//
// MessageQueueResourceSetInstalledEfficiency.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "MessageQueueResourceSetInstalledEfficiency.h"

// ----------------------------------------------------------------------

MessageQueueResourceSetInstalledEfficiency::MessageQueueResourceSetInstalledEfficiency(float installedEfficiency, float tickCount) :
		m_installedEfficiency(installedEfficiency),
		m_tickCount(tickCount)
{
}	

// ----------------------------------------------------------------------

MessageQueueResourceSetInstalledEfficiency::~MessageQueueResourceSetInstalledEfficiency()
{
}

// ======================================================================
