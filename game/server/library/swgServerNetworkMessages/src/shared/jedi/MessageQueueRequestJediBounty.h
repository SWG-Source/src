//========================================================================
//
// MessageQueueRequestJediBounty.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueRequestJediBounty_H
#define INCLUDED_MessageQueueRequestJediBounty_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

/**
 */
class MessageQueueRequestJediBounty : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueRequestJediBounty(const NetworkId & targetId, const NetworkId & hunterId, const std::string & successCallback, const std::string & failCallback, const NetworkId & callbackObjectId);
	virtual ~MessageQueueRequestJediBounty();
	
	MessageQueueRequestJediBounty&	operator=	(const MessageQueueRequestJediBounty & source);
	MessageQueueRequestJediBounty(const MessageQueueRequestJediBounty & source);

	const NetworkId &   getTargetId() const;
	const NetworkId &   getHunterId() const;
	const std::string & getSuccessCallback() const;
	const std::string & getFailCallback() const;
	const NetworkId &   getCallbackObjectId() const;
	
private:
	const NetworkId   m_targetId;
	const NetworkId   m_hunterId;
	const std::string m_successCallback;
	const std::string m_failCallback;
	const NetworkId   m_callbackObjectId;
};


// ======================================================================

inline const NetworkId & MessageQueueRequestJediBounty::getTargetId() const
{
	return m_targetId;
}

inline const NetworkId & MessageQueueRequestJediBounty::getHunterId() const
{
	return m_hunterId;
}

inline const std::string & MessageQueueRequestJediBounty::getSuccessCallback() const
{
	return m_successCallback;
}

inline const std::string & MessageQueueRequestJediBounty::getFailCallback() const
{
	return m_failCallback;
}

inline const NetworkId & MessageQueueRequestJediBounty::getCallbackObjectId() const
{
	return m_callbackObjectId;
}


// ======================================================================


#endif	// INCLUDED_MessageQueueRequestJediBounty_H
