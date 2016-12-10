//========================================================================
//
// MessageQueueResourceWeights.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueResourceWeights_H
#define INCLUDED_MessageQueueResourceWeights_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

#include <vector>

class MemoryBlockManager;

//----------------------------------------------------------------------

class MessageQueueResourceWeights : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit MessageQueueResourceWeights (const std::pair<uint32, uint32> & combinedCrc);
	virtual ~MessageQueueResourceWeights ();

	typedef std::vector<std::pair<int, int> > WeightVector;
	typedef std::vector<std::pair<int, WeightVector> > AttribWeightVector;

	void                          addAssemblyWeight     (int attrib, int slot, int resource, int weight);
	void                          addResourceMaxWeight  (int attrib, int slot, int resource, int weight);

	const AttribWeightVector &    getAllAssemblyWeights    () const;
	const AttribWeightVector &    getAllResourceMaxWeights () const;

	const int                     getAssemblyWeightSlot    (int attrib) const;
	const int                     getResourceMaxWeightSlot (int attrib) const;
	const WeightVector &          getAssemblyWeights       (int attrib) const;
	const WeightVector &          getResourceMaxWeights    (int attrib) const;
	
	const std::pair<uint32, uint32> & getDraftSchematicCrc() const;

private:
	AttribWeightVector            m_assemblyWeights;
	AttribWeightVector            m_resourceMaxWeights;
	std::pair<uint32, uint32>     m_crc;
	std::set<int>              m_attribs;
};

//----------------------------------------------------------------------

inline const int MessageQueueResourceWeights::getAssemblyWeightSlot(int attrib) const
{
	DEBUG_FATAL (attrib < 0 || attrib >= static_cast<int>(m_assemblyWeights.size ()), ("bad index"));
	return m_assemblyWeights[attrib].first;
}

//----------------------------------------------------------------------

inline const int MessageQueueResourceWeights::getResourceMaxWeightSlot(int attrib) const
{
	DEBUG_FATAL (attrib < 0 || attrib >= static_cast<int>(m_assemblyWeights.size ()), ("bad index"));
	return m_resourceMaxWeights[attrib].first;
}

//----------------------------------------------------------------------

inline const MessageQueueResourceWeights::WeightVector & MessageQueueResourceWeights::getAssemblyWeights(int attrib) const
{
	DEBUG_FATAL (attrib < 0 || attrib >= static_cast<int>(m_assemblyWeights.size ()), ("bad index"));
	return m_assemblyWeights[attrib].second;
}

//----------------------------------------------------------------------

inline const MessageQueueResourceWeights::WeightVector & MessageQueueResourceWeights::getResourceMaxWeights(int attrib) const
{
	DEBUG_FATAL (attrib < 0 || attrib >= static_cast<int>(m_assemblyWeights.size ()), ("bad index"));
	return m_resourceMaxWeights[attrib].second;
}

//----------------------------------------------------------------------

inline const MessageQueueResourceWeights::AttribWeightVector & MessageQueueResourceWeights::getAllAssemblyWeights    () const
{
	return m_assemblyWeights;
}

//----------------------------------------------------------------------

inline const MessageQueueResourceWeights::AttribWeightVector & MessageQueueResourceWeights::getAllResourceMaxWeights () const
{
	return m_resourceMaxWeights;
}

//----------------------------------------------------------------------

inline const std::pair<uint32, uint32> & MessageQueueResourceWeights::getDraftSchematicCrc() const
{
	return m_crc;
}

//----------------------------------------------------------------------


#endif	// INCLUDED_MessageQueueResourceWeights_H
