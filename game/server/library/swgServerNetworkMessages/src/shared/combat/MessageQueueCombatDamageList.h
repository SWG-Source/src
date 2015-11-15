//========================================================================
//
// MessageQueueCombatDamageList.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueCombatDamageList_H
#define INCLUDED_MessageQueueCombatDamageList_H

#include "sharedFoundation/MessageQueue.h"

#include "swgSharedUtility/CombatEngineData.h"

#include <vector>
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCombatDamageList : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	         MessageQueueCombatDamageList(void);
	virtual ~MessageQueueCombatDamageList();

	MessageQueueCombatDamageList&	operator=	(const MessageQueueCombatDamageList & source);
	MessageQueueCombatDamageList(const MessageQueueCombatDamageList & source);

	
	void                                              addDamage(const CombatEngineData::DamageData & data);
	CombatEngineData::DamageData &                    addDamage(void);
	const std::vector<CombatEngineData::DamageData> & getDamageList(void) const;

private:
	std::vector<CombatEngineData::DamageData> m_data;
};


inline void MessageQueueCombatDamageList::addDamage(const CombatEngineData::DamageData & data)
{
	m_data.push_back(data);
}	// MessageQueueCombatDamageList::addDamage

inline CombatEngineData::DamageData & MessageQueueCombatDamageList::addDamage(void)
{
	m_data.push_back(CombatEngineData::DamageData());
	return m_data.back();
}

inline const std::vector<CombatEngineData::DamageData> & 
	MessageQueueCombatDamageList::getDamageList(void) const
{
	return m_data;
}	// MessageQueueCombatDamageList::getDamageList


#endif	// INCLUDED_MessageQueueCombatDamageList_H
