//========================================================================
//
// SlowDownEffectMessage.cpp
//
// Copyright 2004, Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SlowDownEffectMessage.h"

// ======================================================================

char const * const SlowDownEffectMessage::MessageType = "SlowDownEffectMessage";

//----------------------------------------------------------------------

SlowDownEffectMessage::SlowDownEffectMessage(const NetworkId & source, const NetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime) :
GameNetworkMessage(MessageType),
m_source(source),
m_target(target),
m_coneLength(coneLength),
m_coneAngle(coneAngle),
m_slopeAngle(slopeAngle),
m_expireTime(expireTime)
{
	addVariable(m_source);
	addVariable(m_target);
	addVariable(m_coneLength);
	addVariable(m_coneAngle);
	addVariable(m_slopeAngle);
	addVariable(m_expireTime);
}

// ----------------------------------------------------------------------

SlowDownEffectMessage::SlowDownEffectMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_source(),
m_target(),
m_coneLength(0),
m_coneAngle(0),
m_slopeAngle(0),
m_expireTime(0)
{
	addVariable(m_source);
	addVariable(m_target);
	addVariable(m_coneLength);
	addVariable(m_coneAngle);
	addVariable(m_slopeAngle);
	addVariable(m_expireTime);

	unpack(source);
}

//----------------------------------------------------------------------
