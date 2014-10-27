//========================================================================
//
// MessageQueueCombatDamageList.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "MessageQueueCombatDamageList.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/AttribModArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedObject/CachedNetworkIdArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCombatDamageList, CM_combatDamageList);

//===================================================================




/**
 * Class constructor.
 */
MessageQueueCombatDamageList::MessageQueueCombatDamageList(void)
{
}	// MessageQueueCombatDamageList::MessageQueueCombatDamageList

/**
 * Class destructor.
 */
MessageQueueCombatDamageList::~MessageQueueCombatDamageList()
{
}	// MessageQueueCombatDamageList::~MessageQueueCombatDamageList


void MessageQueueCombatDamageList::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	uint16 count;
	uint8 temp8;
	
	const MessageQueueCombatDamageList* const msg = safe_cast<const MessageQueueCombatDamageList*> (data);
	
	if (msg)
	{
		const std::vector<CombatEngineData::DamageData> & damageList = 
			msg->getDamageList();
		count = static_cast<uint16>(damageList.size());
		Archive::put(target, count);
		for (std::vector<CombatEngineData::DamageData>::const_iterator
			iter = damageList.begin(); iter != damageList.end(); ++iter)
		{
			Archive::put(target, (*iter).attackerId);
			Archive::put(target, (*iter).weaponId);
			Archive::put(target, (*iter).actionId);
			temp8 = static_cast<uint8>((*iter).damageType);
			Archive::put(target, temp8);
			temp8 = static_cast<uint8>((*iter).hitLocationIndex);
			Archive::put(target, temp8);
			temp8 = static_cast<uint8>((*iter).wounded);
			Archive::put(target, temp8);
			temp8 = static_cast<uint8>((*iter).ignoreInvulnerable);
			Archive::put(target, temp8);
			
			const std::vector<AttribMod::AttribMod> & damage = (*iter).damage;
			count = static_cast<uint16>(damage.size());
			Archive::put(target, count);
			for (std::vector<AttribMod::AttribMod>::const_iterator
				iter2 = damage.begin(); iter2 != damage.end(); ++iter2)
			{
				Archive::put(target, *iter2);
			}
		}
	}
}	// ControllerMessageFactory::packCombatDamageList

//-----------------------------------------------------------------------
MessageQueue::Data* MessageQueueCombatDamageList::unpack(Archive::ReadIterator & source)
{
	CombatEngineData::DamageData damageData;
	uint8 temp8;
	
	MessageQueueCombatDamageList * msg = new MessageQueueCombatDamageList;
	
	uint16 listCount;
	Archive::get(source, listCount);
	for (uint16 i = 0; i < listCount; ++i)
	{
		damageData.damage.clear();
		
		Archive::get(source, damageData.attackerId);
		Archive::get(source, damageData.weaponId);
		Archive::get(source, damageData.actionId);
		Archive::get(source, temp8);
		damageData.damageType = static_cast<CombatEngineData::DamageType>(temp8);
		Archive::get(source, temp8);
		damageData.hitLocationIndex = temp8;
		Archive::get(source, temp8);
		damageData.wounded = temp8 != 0;
		Archive::get(source, temp8);
		damageData.ignoreInvulnerable = temp8 != 0;
		
		uint16 damageCount;
		Archive::get(source, damageCount);
		for (uint16 j = 0; j < damageCount; ++j)
		{
			damageData.damage.push_back(AttribMod::AttribMod());
			Archive::get(source, damageData.damage.back());
		}
		
		msg->addDamage(damageData);
	}
	
	return msg;
}	// ControllerMessageFactory::unpackCombatAction

//-----------------------------------------------------------------------

