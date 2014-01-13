//========================================================================
//
// MessageQueueOpponentInfo.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueOpponentInfo_H
#define INCLUDED_MessageQueueOpponentInfo_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class MessageQueueOpponentInfo : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueOpponentInfo();
	virtual ~MessageQueueOpponentInfo();

	MessageQueueOpponentInfo&	operator=	(const MessageQueueOpponentInfo & source);
	MessageQueueOpponentInfo(const MessageQueueOpponentInfo & source);

	void setOpponent(const NetworkId & opponent);
	void setAttributes(int health, int action, int mind);

	const NetworkId & getOpponent(void) const;
	uint16 getHealth(void) const;
	uint16 getAction(void) const;
	uint16 getMind(void) const;
 
private:
	NetworkId m_opponent;
	uint16    m_health;
	uint16    m_action;
	uint16    m_mind;
};


inline MessageQueueOpponentInfo::MessageQueueOpponentInfo()
{
}

inline MessageQueueOpponentInfo::MessageQueueOpponentInfo(
	const MessageQueueOpponentInfo & source) :
	MessageQueue::Data(source),
	m_opponent(source.m_opponent),
	m_health(source.m_health),
	m_action(source.m_action),
	m_mind(source.m_mind)
{
}

inline MessageQueueOpponentInfo & MessageQueueOpponentInfo::operator=(
	const MessageQueueOpponentInfo & source)
{
	m_opponent = source.m_opponent;
	m_health = source.m_health;
	m_action = source.m_action;
	m_mind = source.m_mind;
	return *this;
}

inline void MessageQueueOpponentInfo::setOpponent(const NetworkId & opponent)
{
	m_opponent = opponent;
}

inline void MessageQueueOpponentInfo::setAttributes(int health, int action, int mind)
{
	m_health = static_cast<uint16>(health);
	m_action = static_cast<uint16>(action);
	m_mind = static_cast<uint16>(mind);
}

inline const NetworkId & MessageQueueOpponentInfo::getOpponent(void) const
{
	return m_opponent;
}

inline uint16 MessageQueueOpponentInfo::getHealth(void) const
{
	return m_health;
}

inline uint16 MessageQueueOpponentInfo::getAction(void) const
{
	return m_action;
}

inline uint16 MessageQueueOpponentInfo::getMind(void) const
{
	return m_mind;
}


#endif	// INCLUDED_MessageQueueOpponentInfo_H
