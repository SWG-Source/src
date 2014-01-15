//========================================================================
//
// MessageQueueGrantOfflineXp.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueGrantOfflineXp_H
#define INCLUDED_MessageQueueGrantOfflineXp_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

/**
 */
class MessageQueueGrantOfflineXp : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueGrantOfflineXp(const NetworkId & player, const std::string & experienceType, int amount);
	virtual ~MessageQueueGrantOfflineXp();
	
	MessageQueueGrantOfflineXp&	operator=	(const MessageQueueGrantOfflineXp & source);
	MessageQueueGrantOfflineXp(const MessageQueueGrantOfflineXp & source);

	const NetworkId &       getPlayerId() const;
	const std::string &     getXpType() const;
	int                     getXpAmount() const;
	
private:
	const NetworkId   m_playerId;
	const std::string m_xpType;
	const int         m_xpAmount;
};

// ======================================================================

inline const NetworkId & MessageQueueGrantOfflineXp::getPlayerId() const
{
	return m_playerId;
}

inline const std::string & MessageQueueGrantOfflineXp::getXpType() const
{
	return m_xpType;
}

inline int MessageQueueGrantOfflineXp::getXpAmount() const
{
	return m_xpAmount;
}


// ======================================================================


#endif	// INCLUDED_MessageQueueGrantOfflineXp_H
