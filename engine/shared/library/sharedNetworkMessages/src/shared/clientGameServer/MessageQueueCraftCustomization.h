//========================================================================
//
// MessageQueueCraftCustomization.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftCustomization_H
#define INCLUDED_MessageQueueCraftCustomization_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/CraftingData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCraftCustomization : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit MessageQueueCraftCustomization(void);
	virtual ~MessageQueueCraftCustomization();

	const Unicode::String &                    getName(void) const;
	int                                        getAppearance(void) const;
	const std::vector<Crafting::CustomValue> & getCustomization(void) const;
	int                                        getItemCount(void) const;

	void setName(const Unicode::String & name);
	void setAppearance(int appearance);
	void addCustomization(int property, int value);
	void setItemCount(int count);

private:
	Unicode::String                    m_name;
	int                                m_appearance;
	std::vector<Crafting::CustomValue> m_customization;
	int                                m_itemCount;
};


//----------------------------------------------------------------------

inline MessageQueueCraftCustomization::MessageQueueCraftCustomization(void) :
	MessageQueue::Data(),
	m_name(),
	m_appearance(-1),
	m_customization(),
	m_itemCount(1)
{
}

inline const Unicode::String & MessageQueueCraftCustomization::getName(void) const
{
	return m_name;
}

inline int MessageQueueCraftCustomization::getAppearance(void) const
{
	return m_appearance;
}

inline const std::vector<Crafting::CustomValue> & MessageQueueCraftCustomization::getCustomization(void) const
{
	return m_customization;
}

inline int MessageQueueCraftCustomization::getItemCount(void) const
{
	return m_itemCount;
}

inline void MessageQueueCraftCustomization::setName(const Unicode::String & name)
{
	m_name = name;
}

inline void MessageQueueCraftCustomization::setAppearance(int appearance)
{
	m_appearance = appearance;
}

inline void MessageQueueCraftCustomization::addCustomization(int property, int value)
{
	m_customization.push_back(Crafting::CustomValue(property, value));
}

inline void MessageQueueCraftCustomization::setItemCount(int count)
{
	m_itemCount = count;
}

//----------------------------------------------------------------------


#endif	// INCLUDED_MessageQueueCraftCustomization_H
