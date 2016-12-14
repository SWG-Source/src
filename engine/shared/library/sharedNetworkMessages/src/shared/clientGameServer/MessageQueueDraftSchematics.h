//========================================================================
//
// MessageQueueDraftSchematics.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueDraftSchematics_H
#define INCLUDED_MessageQueueDraftSchematics_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematicsData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//----------------------------------------------------------------------

// @todo: can we get by with a single category value for the entire message?
class MessageQueueDraftSchematics : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	typedef MessageQueueDraftSchematicsData SchematicData;

	typedef std::vector<SchematicData> SchematicVector;

public:

	         MessageQueueDraftSchematics(const NetworkId & toolId, const NetworkId & stationId);
	virtual ~MessageQueueDraftSchematics();

	void                    addSchematic(const std::pair<uint32, uint32> & crc, int category);
	int                     getSchematicCount(void) const;
	const SchematicData &   getSchematic(int index) const;
	const SchematicVector & getSchematics () const;
	const NetworkId &       getToolId () const;
	const NetworkId &       getStationId () const;

private:
	SchematicVector         m_schematics;
	NetworkId               m_toolId;
	NetworkId               m_stationId;
};

//----------------------------------------------------------------------

inline const MessageQueueDraftSchematics::SchematicVector & MessageQueueDraftSchematics::getSchematics () const
{
	return m_schematics;
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueDraftSchematics::getToolId () const
{
	return m_toolId;
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueDraftSchematics::getStationId () const
{
	return m_stationId;
}

//----------------------------------------------------------------------


#endif	// INCLUDED_MessageQueueDraftSchematics_H
