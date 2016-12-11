//======================================================================
//
// MessageQueueHarvesterResourceData.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueHarvesterResourceData_H
#define INCLUDED_MessageQueueHarvesterResourceData_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/InstallationResourceData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A generic message queue data that contains a simple payload of one NetworkId.
* The meaning of the NetworkId is context and message dependant.
*
*/

class MessageQueueHarvesterResourceData : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	typedef std::vector<InstallationResourceData> DataVector;

	                        MessageQueueHarvesterResourceData (const NetworkId & id, const DataVector & data);
	virtual                ~MessageQueueHarvesterResourceData();

	const NetworkId &       getHarvesterId () const;
	const DataVector &      getDataVector () const;

private:

	MessageQueueHarvesterResourceData&  operator= (const MessageQueueHarvesterResourceData & source);
	MessageQueueHarvesterResourceData(const MessageQueueHarvesterResourceData & source);

	NetworkId  m_harvesterId;
	DataVector m_data;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueHarvesterResourceData::getHarvesterId(void) const
{
	return m_harvesterId;
}

//----------------------------------------------------------------------

inline const MessageQueueHarvesterResourceData::DataVector &      MessageQueueHarvesterResourceData::getDataVector () const
{
	return m_data;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueHarvesterResourceData_H

