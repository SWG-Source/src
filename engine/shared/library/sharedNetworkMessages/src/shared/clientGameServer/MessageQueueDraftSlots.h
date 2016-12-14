//========================================================================
//
// MessageQueueDraftSlots.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#ifndef INCLUDED_MessageQueueDraftSlots_H
#define INCLUDED_MessageQueueDraftSlots_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//----------------------------------------------------------------------

class MessageQueueDraftSlots : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	typedef MessageQueueDraftSlotsData::Option Option;
	typedef MessageQueueDraftSlotsData Slot;

public:

	explicit MessageQueueDraftSlots(const NetworkId & toolId, const NetworkId & manfSchemId);
	virtual ~MessageQueueDraftSlots();

	const NetworkId &  getToolId(void) const;

	void               addSlot(const Slot & slot);
	int                getSlotCount(void) const;
	const Slot  &      getSlot(int index) const;

	typedef std::vector<Slot> SlotVector;
	const SlotVector & getSlots() const;
	void               setSlots(const SlotVector & sv);

	void               setManfSchemId(const NetworkId & id);
	const NetworkId &  getManfSchemId() const;

	void               setPrototypeId(const NetworkId & id);
	const NetworkId &  getPrototypeId() const;

	void               setVolume(int volume);
	int                getVolume() const;

	void               setCanManufacture(bool canManufacture);
	bool               canManufacture() const;

private:
	std::vector<Slot>  m_slots;
	NetworkId          m_toolId;
	NetworkId          m_manfSchemId;
	NetworkId          m_prototypeId;
	int                m_volume;
	bool               m_canManufacture;
};

//----------------------------------------------------------------------

inline MessageQueueDraftSlots::MessageQueueDraftSlots(const NetworkId & toolId, const NetworkId & manfSchemId) :
	m_slots(),
	m_toolId(toolId),
	m_manfSchemId(manfSchemId),
	m_prototypeId(),
	m_volume(1),
	m_canManufacture()
{
}

//----------------------------------------------------------------------

inline const NetworkId & MessageQueueDraftSlots::getToolId(void) const
{
	return m_toolId;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::addSlot(const MessageQueueDraftSlots::Slot & slot)
{
	m_slots.push_back(slot);
}

//----------------------------------------------------------------------

inline int MessageQueueDraftSlots::getSlotCount(void) const
{
	return m_slots.size();
}

//----------------------------------------------------------------------

inline const MessageQueueDraftSlots::Slot & MessageQueueDraftSlots::getSlot(int index) const
{
	DEBUG_FATAL(index < 0 || index >= static_cast<int>(m_slots.size()), ("MessageQueueDraftSlots::getSlot index out of range"));
	return m_slots.at(index);
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::setSlots(const SlotVector & sv)
{
	m_slots = sv;
}

//----------------------------------------------------------------------

inline const MessageQueueDraftSlots::SlotVector & MessageQueueDraftSlots::getSlots() const
{
	return m_slots;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::setManfSchemId(const NetworkId & id)
{
	m_manfSchemId = id;
}

//----------------------------------------------------------------------

inline const NetworkId &  MessageQueueDraftSlots::getManfSchemId() const
{
	return m_manfSchemId;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::setPrototypeId(const NetworkId & id)
{
	m_prototypeId = id;
}

//----------------------------------------------------------------------

inline const NetworkId &  MessageQueueDraftSlots::getPrototypeId() const
{
	return m_prototypeId;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::setVolume(int volume)
{
	m_volume = volume;
}

//----------------------------------------------------------------------

inline int MessageQueueDraftSlots::getVolume() const
{
	return m_volume;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlots::setCanManufacture(bool canManufacture)
{
	m_canManufacture = canManufacture;
}

//----------------------------------------------------------------------

inline bool MessageQueueDraftSlots::canManufacture() const
{
	return m_canManufacture;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueDraftSlots_H
