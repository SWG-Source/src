//========================================================================
//
// MessageQueueResourceWeights.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"


//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueResourceWeights, CM_resourceWeights);

//===================================================================

//----------------------------------------------------------------------

MessageQueueResourceWeights::MessageQueueResourceWeights(const std::pair<uint32, uint32> & combinedCrc) :
MessageQueue::Data    (),
m_assemblyWeights     (),
m_resourceMaxWeights  (),
m_crc                 (combinedCrc),
m_attribs             ()
{
}

/**
 * Class destructor.
 */
MessageQueueResourceWeights::~MessageQueueResourceWeights()
{
}

//----------------------------------------------------------------------

/**
 * Adds a resource weight to the assembly weights list.
 *
 * @param attrib		the attribute the weight belongs to
 * @param slot          the slot associated with the attribute (-1 = all slots)
 * @param resource		the resource attribute id
 * @param weight		the resource weight
 */
void MessageQueueResourceWeights::addAssemblyWeight(int attrib, int slot, int resource,
	int weight)
{
	if (static_cast<int>(m_assemblyWeights.size()) <= attrib)
		m_assemblyWeights.resize(attrib + 1);

	m_attribs.insert(attrib);
	std::pair<int, WeightVector> & weights = m_assemblyWeights[attrib];
	weights.first = slot;
	weights.second.push_back(std::make_pair(resource, weight));
}	// MessageQueueResourceWeights::addAssemblyWeight

//----------------------------------------------------------------------

/**
 * Adds a resource weight to the resource max weights list.
 *
 * @param attrib		the attribute the weight belongs to
 * @param slot          the slot associated with the attribute (-1 = all slots)
 * @param resource		the resource attribute id
 * @param weight		the resource weight
 */
void MessageQueueResourceWeights::addResourceMaxWeight(int attrib, int slot,
	int resource, int weight)
{
	if (static_cast<int>(m_resourceMaxWeights.size()) <= attrib)
		m_resourceMaxWeights.resize(attrib + 1);

	std::pair<int, WeightVector> & weights = m_resourceMaxWeights[attrib];
	weights.first = slot;
	weights.second.push_back(std::make_pair(resource, weight));
}	// MessageQueueResourceWeights::addResourceMaxWeight

//----------------------------------------------------------------------

void MessageQueueResourceWeights::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueResourceWeights* const msg = safe_cast<const MessageQueueResourceWeights*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_crc);

		int8  idata = 0;
		uint8 udata = 0;

		size_t attribCount = msg->m_attribs.size();
		if (attribCount > msg->m_assemblyWeights.size() || msg->m_assemblyWeights.size() != msg->m_resourceMaxWeights.size())
		{
			WARNING(true, ("MessageQueueResourceWeights::pack has mismatching "
				"weight arrays! Attrib count = %u, assembly weights = %u, "
				"resource weights = %u", attribCount, msg->m_assemblyWeights.size(),
				msg->m_resourceMaxWeights.size()));
			udata = 0;
			Archive::put(target, udata);	
			return;
		}

		udata = static_cast<uint8>(attribCount);
		Archive::put(target, udata);

		// write m_assemblyWeights info
		int assemblyCount = 0;
		int resourceCount = 0;
		{
			int count = msg->m_assemblyWeights.size();
			for (int i = 0; i < count; ++i)
			{
				const std::pair<int, WeightVector> & weights = msg->m_assemblyWeights[i];
				const int weightCount = weights.second.size();
				if (weightCount == 0)
					continue;
				++assemblyCount;
				// write attribute index
				udata = static_cast<uint8>(i);
				Archive::put (target, udata);
				// write the attribute slot
				idata = static_cast<int8>(weights.first);
				Archive::put (target, idata);
				// write the assembly weights
				udata = static_cast<uint8>(weightCount);
				Archive::put (target, udata);
				for (int j = 0; j < weightCount; ++j)
				{
					const std::pair<int, int> & weight = weights.second[j];
					udata = static_cast<uint8>(((weight.first << 4) & 0x00f0) | 
						(weight.second & 0x000f));
					Archive::put (target, udata);
				}
			}
		}

		// write m_resourceMaxWeights info
		{
			int count = msg->m_resourceMaxWeights.size();
			for (int i = 0; i < count; ++i)
			{
				const std::pair<int, WeightVector> & weights = msg->m_resourceMaxWeights[i];
				const int weightCount = weights.second.size();
				if (weightCount == 0)
					continue;
				++resourceCount;
				// write attribute index
				udata = static_cast<uint8>(i);
				Archive::put (target, udata);
				// write the attribute slot
				idata = static_cast<int8>(weights.first);
				Archive::put (target, idata);
				// write the assembly weights
				udata = static_cast<uint8>(weightCount);
				Archive::put (target, udata);
				for (int j = 0; j < weightCount; ++j)
				{
					const std::pair<int, int> & weight = weights.second[j];
					udata = static_cast<uint8>(((weight.first << 4) & 0x00f0) | 
						(weight.second & 0x000f));
					Archive::put (target, udata);
				}
			}
		}

		int missingCount = attribCount * 2 - assemblyCount - resourceCount;
		if (missingCount > 0)
		{
			WARNING(true, ("MessageQueueResourceWeights::pack for schematic "
				"template crc %u has mismatching weight counts! Attrib count = %d, "
				"assembly count = %d, resource count = %d", msg->m_crc.first,
				attribCount, assemblyCount, resourceCount));
			// fill in the byte stream with some dummy data
			for (int i = 0; i < missingCount; ++i)
			{
				udata = static_cast<uint8>(-1);
				Archive::put (target, udata);
				idata = static_cast<int8>(-1);
				Archive::put (target, idata);
				udata = static_cast<uint8>(0);
				Archive::put (target, udata);
			}
		}
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueResourceWeights::unpack(Archive::ReadIterator & source)
{
	UNREF(source);

	std::pair<uint32, uint32> crc;
	Archive::get(source, crc);

	MessageQueueResourceWeights * const msg = new MessageQueueResourceWeights(crc);

	int8  idata = 0;
	uint8 udata = 0;

	Archive::get(source, udata);
	int attribCount = udata;

	// read m_assemblyWeights info
	{
		for (int i = 0; i < attribCount; ++i)
		{
			Archive::get(source, udata);
			const int attribute = udata;
			Archive::get(source, idata);
			const int slot = idata;
			Archive::get(source, udata);
			const int weightCount = udata;
			for (int j = 0; j < weightCount; ++j)
			{
				Archive::get(source, udata);
				int resource = ((udata & 0xf0) >> 4);
				int weight = (udata & 0x0f);
				msg->addAssemblyWeight(attribute, slot, resource, weight);
			}
		}
	}

	// read m_resourceMaxWeights info
	{
		for (int i = 0; i < attribCount; ++i)
		{
			Archive::get(source, udata);
			const int attribute = udata;
			Archive::get(source, idata);
			const int slot = idata;
			Archive::get(source, udata);
			const int weightCount = udata;
			for (int j = 0; j < weightCount; ++j)
			{
				Archive::get(source, udata);
				int resource = ((udata & 0xf0) >> 4);
				int weight = (udata & 0x0f);
				msg->addResourceMaxWeight(attribute, slot, resource, weight);
			}
		}
	}

	return msg;
}
//-----------------------------------------------------------------------

