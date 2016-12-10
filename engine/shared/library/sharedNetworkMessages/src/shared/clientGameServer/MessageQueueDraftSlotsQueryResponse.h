//======================================================================
//
// MessageQueueDraftSlotsQueryResponse.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSlotsQueryResponse_H
#define INCLUDED_MessageQueueDraftSlotsQueryResponse_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//----------------------------------------------------------------------

class MessageQueueDraftSlotsQueryResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	typedef MessageQueueDraftSlotsData::Option Option;
	typedef MessageQueueDraftSlotsData         Slot;
	typedef std::vector<Slot>               SlotVector;

public:

	explicit MessageQueueDraftSlotsQueryResponse (const std::pair<uint32, uint32> & combinedCrc);
	virtual ~MessageQueueDraftSlotsQueryResponse ();

	void                  setComplexity(int complexity);
	void                  setVolume (int volume);
	void                  setCanManufacture(bool canManufacture);
	void                  addSlot(const Slot & slot);

	const std::pair<uint32, uint32> & getDraftSchematicCrc   () const;
	int                               getComplexity          () const;
	int                               getVolume              () const;
	bool                              canManufacture         () const;
	const SlotVector &                getSlots               () const;
	void                              setSlots               (const SlotVector & sv);

private:
	std::pair<uint32, uint32> m_crc;
	SlotVector                m_slots;
	int                       m_complexity;
	int                       m_volume;
	bool                      m_canManufacture;
};

//----------------------------------------------------------------------

inline void MessageQueueDraftSlotsQueryResponse::setComplexity(int complexity)
{
	m_complexity = complexity;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlotsQueryResponse::setVolume (int volume)
{
	m_volume = volume;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlotsQueryResponse::setCanManufacture(bool canManufacture)
{
	m_canManufacture = canManufacture;
}

//----------------------------------------------------------------------

inline void MessageQueueDraftSlotsQueryResponse::addSlot(const MessageQueueDraftSlotsQueryResponse::Slot & slot)
{
	m_slots.push_back(slot);
}

//----------------------------------------------------------------------

inline const std::pair<uint32, uint32> & MessageQueueDraftSlotsQueryResponse::getDraftSchematicCrc  () const
{
	return m_crc;
}

//----------------------------------------------------------------------

inline int MessageQueueDraftSlotsQueryResponse::getComplexity() const
{
	return m_complexity;
}

//----------------------------------------------------------------------

inline int MessageQueueDraftSlotsQueryResponse::getVolume () const
{
	return m_volume;
}

//----------------------------------------------------------------------

inline bool MessageQueueDraftSlotsQueryResponse::canManufacture() const
{
	return m_canManufacture;
}

//----------------------------------------------------------------------

inline const MessageQueueDraftSlotsQueryResponse::SlotVector & MessageQueueDraftSlotsQueryResponse::getSlots () const
{
	return m_slots;
}

//======================================================================

#endif
