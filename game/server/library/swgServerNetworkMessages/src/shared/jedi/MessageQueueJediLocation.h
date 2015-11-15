//========================================================================
//
// MessageQueueJediLocation.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueJediLocation_H
#define INCLUDED_MessageQueueJediLocation_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"


class MemoryBlockManager;


/**
 */
class MessageQueueJediLocation : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueJediLocation(const NetworkId & id, const Vector & location, const std::string & scene);
	virtual ~MessageQueueJediLocation();
	
	MessageQueueJediLocation&	operator=	(const MessageQueueJediLocation & source);
	MessageQueueJediLocation(const MessageQueueJediLocation & source);

	const NetworkId &       getId() const;
	const Vector &          getLocation() const;
	const std::string &     getScene() const;
	
private:
	const NetworkId       m_id;
	const Vector          m_location;
	const std::string     m_scene;
};

// ======================================================================

inline const NetworkId & MessageQueueJediLocation::getId() const
{
	return m_id;
}

inline const Vector & MessageQueueJediLocation::getLocation() const
{
	return m_location;
}

inline const std::string & MessageQueueJediLocation::getScene() const
{
	return m_scene;
}


// ======================================================================


#endif	// INCLUDED_MessageQueueJediLocation_H
