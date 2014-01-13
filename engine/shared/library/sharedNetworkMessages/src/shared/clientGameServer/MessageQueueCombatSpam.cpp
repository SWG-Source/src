//======================================================================
//
// MessageQueueCombatSpam.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"

#include "sharedFoundation/NetworkId.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueCombatSpamArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueCombatSpam, CM_combatSpam);


//===================================================================

MessageQueueCombatSpam::MessageQueueCombatSpam(const NetworkId & attacker, const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const NetworkId & weapon, const NetworkId & armor, const StringId & attackName, int rawDamage, int damageType, int elementalDamage, int elementalDamageType, int bleedDamage, int critDamage, int blockedDamage, int finalDamage, int hitLocation, bool success, bool critical, bool glancing, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, bool proc, int spamType) :
	MessageQueue::Data(),
	m_attacker(attacker),
	m_attackerPosition_w(attackerPosition_w),
	m_defender(defender),
	m_defenderPosition_w(defenderPosition_w),
	m_weapon(weapon),
	m_weaponName(),
	m_armor(armor),
	m_attackName(attackName),
	m_rawDamage(rawDamage),
	m_damageType(damageType),
	m_elementalDamage(elementalDamage),
	m_elementalDamageType(elementalDamageType),
	m_bleedDamage(bleedDamage),
	m_critDamage(critDamage),
	m_blockedDamage(blockedDamage),
	m_finalDamage(finalDamage),
	m_hitLocation(hitLocation),
	m_success(success),
	m_critical(critical),
	m_glancing(glancing),
	m_crushing(crushing),
	m_strikethrough(strikethrough),
	m_strikethroughAmmount(strikethroughAmmount),
	m_evadeResult(evadeResult),
	m_evadeAmmount(evadeAmmount),
	m_blockResult(blockResult),
	m_block(block),
	m_dodge(dodge),
	m_parry(parry),
	m_proc(proc),
	m_spamMessage(),
	m_spamType(spamType)
{
}

//----------------------------------------------------------------------

MessageQueueCombatSpam::MessageQueueCombatSpam(const NetworkId & attacker, const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const StringId & weaponName, const NetworkId & armor, const StringId & attackName, int rawDamage, int damageType, int elementalDamage, int elementalDamageType, int bleedDamage, int critDamage, int blockedDamage, int finalDamage, int hitLocation, bool success, bool critical, bool glancing, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, bool proc, int spamType) :
	MessageQueue::Data(),
	m_attacker(attacker),
	m_attackerPosition_w(attackerPosition_w),
	m_defender(defender),
	m_defenderPosition_w(defenderPosition_w),
	m_weapon(),
	m_weaponName(weaponName),
	m_armor(armor),
	m_attackName(attackName),
	m_rawDamage(rawDamage),
	m_damageType(damageType),
	m_elementalDamage(elementalDamage),
	m_elementalDamageType(elementalDamageType),
	m_bleedDamage(bleedDamage),
	m_critDamage(critDamage),
	m_blockedDamage(blockedDamage),
	m_finalDamage(finalDamage),
	m_hitLocation(hitLocation),
	m_success(success),
	m_critical(critical),
	m_glancing(glancing),
	m_crushing(crushing),
	m_strikethrough(strikethrough),
	m_strikethroughAmmount(strikethroughAmmount),
	m_evadeResult(evadeResult),
	m_evadeAmmount(evadeAmmount),
	m_blockResult(blockResult),
	m_block(block),
	m_dodge(dodge),
	m_parry(parry),
	m_proc(proc),
	m_spamMessage(),
	m_spamType(spamType)
{
}

//----------------------------------------------------------------------

MessageQueueCombatSpam::MessageQueueCombatSpam(const Unicode::String & spamMessage, bool critical, bool glancing, bool proc, int spamType) :
	MessageQueue::Data (),
	m_attacker(),
	m_attackerPosition_w(),
	m_defender(),
	m_defenderPosition_w(),
	m_weapon(),
	m_weaponName(),
	m_armor(),
	m_attackName(),
	m_rawDamage(0),
	m_damageType(0),
	m_elementalDamage(0),
	m_elementalDamageType(0),
	m_bleedDamage(0),
	m_critDamage(0),
	m_blockedDamage(0),
	m_finalDamage(0),
	m_hitLocation(0),
	m_success(false),
	m_critical(critical),
	m_glancing(glancing),
	m_crushing(false),
	m_strikethrough(false),
	m_strikethroughAmmount(0.0f),
	m_evadeResult(false),
	m_evadeAmmount(0.0f),
	m_blockResult(false),
	m_block(0),
	m_dodge(false),
	m_parry(false),
	m_proc(proc),
	m_spamMessage(spamMessage),
	m_spamType(spamType)
{
}

//----------------------------------------------------------------------

MessageQueueCombatSpam::MessageQueueCombatSpam(const NetworkId & attacker, const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const Unicode::String & spamMessage, bool critical, bool glancing, bool proc, int spamType) :
	MessageQueue::Data (),
	m_attacker(attacker),
	m_attackerPosition_w(attackerPosition_w),
	m_defender(defender),
	m_defenderPosition_w(defenderPosition_w),
	m_weapon(),
	m_weaponName(),
	m_armor(),
	m_attackName(),
	m_rawDamage(0),
	m_damageType(0),
	m_elementalDamage(0),
	m_elementalDamageType(0),
	m_bleedDamage(0),
	m_critDamage(0),
	m_blockedDamage(0),
	m_finalDamage(0),
	m_hitLocation(0),
	m_success(false),
	m_critical(critical),
	m_glancing(glancing),
	m_crushing(false),
	m_strikethrough(false),
	m_strikethroughAmmount(0.0f),
	m_evadeResult(false),
	m_evadeAmmount(0.0f),
	m_blockResult(false),
	m_block(0),
	m_dodge(false),
	m_parry(false),
	m_proc(proc),
	m_spamMessage(spamMessage),
	m_spamType(spamType)
{
}

//----------------------------------------------------------------------

MessageQueueCombatSpam::MessageQueueCombatSpam (const MessageQueueCombatSpam& rhs) :
	MessageQueue::Data (),
	m_attacker(rhs.m_attacker),
	m_attackerPosition_w(rhs.m_attackerPosition_w),
	m_defender(rhs.m_defender),
	m_defenderPosition_w(rhs.m_defenderPosition_w),
	m_weapon(rhs.m_weapon),
	m_weaponName(rhs.m_weaponName),
	m_armor(rhs.m_armor),
	m_attackName(rhs.m_attackName),
	m_rawDamage(rhs.m_rawDamage),
	m_damageType(rhs.m_damageType),
	m_elementalDamage(rhs.m_elementalDamage),
	m_elementalDamageType(rhs.m_elementalDamageType),
	m_bleedDamage(rhs.m_bleedDamage),
	m_critDamage(rhs.m_critDamage),
	m_blockedDamage(rhs.m_blockedDamage),
	m_finalDamage(rhs.m_finalDamage),
	m_hitLocation(rhs.m_hitLocation),
	m_success(rhs.m_success),
	m_critical(rhs.m_critical),
	m_glancing(rhs.m_glancing),
	m_crushing(rhs.m_crushing),
	m_strikethrough(rhs.m_strikethrough),
	m_strikethroughAmmount(rhs.m_strikethroughAmmount),
	m_evadeResult(rhs.m_evadeResult),
	m_evadeAmmount(rhs.m_evadeAmmount),
	m_blockResult(rhs.m_blockResult),
	m_block(rhs.m_block),
	m_dodge(rhs.m_dodge),
	m_parry(rhs.m_parry),
	m_proc(rhs.m_proc),
	m_spamMessage(rhs.m_spamMessage),
	m_spamType(rhs.m_spamType)
{
}

//======================================================================
