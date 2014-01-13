//========================================================================
//
// SlowDownEffectMessage.h
//
// Copyright 2004, Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#ifndef	INCLUDED_SlowDownEffectMessage_H
#define	INCLUDED_SlowDownEffectMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class SlowDownEffectMessage : public GameNetworkMessage
{
public:

	static char const * const MessageType;

	         SlowDownEffectMessage(const NetworkId & source, const NetworkId & target, float coneLength, float coneAngle, float slopeAngle, unsigned long expireTime);
	explicit SlowDownEffectMessage(Archive::ReadIterator & source);

public:

	const NetworkId & getSource() const;
	const NetworkId & getTarget() const;
	float getConeLength() const;
	float getConeAngle() const;
	float getSlopeAngle() const;
	unsigned long getExpireTime() const;

private: 
	Archive::AutoVariable<NetworkId>  m_source;
	Archive::AutoVariable<NetworkId>  m_target;
	Archive::AutoVariable<float>      m_coneLength; 
	Archive::AutoVariable<float>      m_coneAngle;
	Archive::AutoVariable<float>      m_slopeAngle;
	Archive::AutoVariable<unsigned long>      m_expireTime;
};

//-----------------------------------------------------------------------

inline const NetworkId & SlowDownEffectMessage::getSource() const
{
	return m_source.get();
}

inline const NetworkId & SlowDownEffectMessage::getTarget() const
{
	return m_target.get();
}

inline float SlowDownEffectMessage::getConeLength() const
{
	return m_coneLength.get();
}

inline float SlowDownEffectMessage::getConeAngle() const
{
	return m_coneAngle.get();
}

inline float SlowDownEffectMessage::getSlopeAngle() const
{
	return m_slopeAngle.get();
}

inline unsigned long SlowDownEffectMessage::getExpireTime() const
{
	return m_expireTime.get();
}

//-----------------------------------------------------------------------

#endif // INCLUDED_SlowDownEffectMessage_H
