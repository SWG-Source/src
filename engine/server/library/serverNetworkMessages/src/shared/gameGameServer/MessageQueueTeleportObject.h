// ======================================================================
//
// MessageQueueTeleportObject.h
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueTeleportObject_H
#define INCLUDED_MessageQueueTeleportObject_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

// ======================================================================

class MemoryBlockManager;

// ======================================================================

class MessageQueueTeleportObject : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueTeleportObject(Vector const &position_w, NetworkId const &targetContainerId, std::string const &targetCellName, Vector const &position_p, std::string const &scriptCallback);
	virtual ~MessageQueueTeleportObject();

	Vector const &getPosition_w() const;
	NetworkId const &getTargetContainerId() const;
	std::string const &getTargetCellName() const;
	Vector const &getPosition_p() const;
	std::string const &getScriptCallback() const;

private:
	MessageQueueTeleportObject(MessageQueueTeleportObject const &);
	MessageQueueTeleportObject &operator=(MessageQueueTeleportObject const &);

private:
	Vector m_position_w;
	NetworkId m_targetContainerId;
	std::string m_targetCellName;
	Vector m_position_p;
	std::string m_scriptCallback;
};

// ----------------------------------------------------------------------

inline Vector const &MessageQueueTeleportObject::getPosition_w() const
{
	return m_position_w;
}

// ----------------------------------------------------------------------

inline NetworkId const &MessageQueueTeleportObject::getTargetContainerId() const
{
	return m_targetContainerId;
}

// ----------------------------------------------------------------------

inline std::string const &MessageQueueTeleportObject::getTargetCellName() const
{
	return m_targetCellName;
}

// ----------------------------------------------------------------------

inline Vector const &MessageQueueTeleportObject::getPosition_p() const
{
	return m_position_p;
}

// ----------------------------------------------------------------------

inline std::string const &MessageQueueTeleportObject::getScriptCallback() const
{
	return m_scriptCallback;
}

// ======================================================================

#endif	// INCLUDED_MessageQueueTeleportObject_H

