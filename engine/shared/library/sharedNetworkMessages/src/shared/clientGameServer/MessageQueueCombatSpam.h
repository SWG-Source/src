//======================================================================
//
// MessageQueueCombatSpam.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueCombatSpam_H
#define INCLUDED_MessageQueueCombatSpam_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"
#include "Unicode.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

/**
 * A CombatSpam message is issued on an server object's MessageQueue if the object
 * observes 'source' performing CombatSpam towards 'target'.
 */

//----------------------------------------------------------------------

class MessageQueueCombatSpam : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	MessageQueueCombatSpam (const NetworkId & attacker,	const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const NetworkId & weapon, const NetworkId & armor, const StringId & attackName, int rawDamage, int damageType, int elementalDamage, int elementalDamageType, int bleedDamage, int critDamage, int blockedDamage, int finalDamage, int hitLocation, bool success, bool critical, bool glancing, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, bool proc, int spamType);
	MessageQueueCombatSpam (const NetworkId & attacker,	const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const StringId & weaponName, const NetworkId & armor, const StringId & attackName, int rawDamage, int damageType, int elementalDamage, int elementalDamageType, int bleedDamage, int critDamage, int blockedDamage, int finalDamage, int hitLocation, bool success, bool critical, bool glancing, bool crushing, bool strikethrough, float strikethroughAmmount, bool evadeResult, float evadeAmmount, bool blockResult, int block, bool dodge, bool parry, bool proc, int spamType);
	MessageQueueCombatSpam (const Unicode::String & spamMessage, bool critical, bool glancing, bool proc, int spamType);
	MessageQueueCombatSpam (const NetworkId & attacker,	const Vector & attackerPosition_w, const NetworkId & defender, const Vector & defenderPosition_w, const Unicode::String & spamMessage, bool critical, bool glancing, bool proc, int spamType);
	MessageQueueCombatSpam (const MessageQueueCombatSpam&);

private:

	MessageQueueCombatSpam ();
	MessageQueueCombatSpam& operator= (const MessageQueueCombatSpam&);

public:

	const NetworkId m_attacker;
	const Vector    m_attackerPosition_w;
	const NetworkId m_defender;
	const Vector    m_defenderPosition_w;
	const NetworkId m_weapon;
	const StringId  m_weaponName;
	const NetworkId m_armor;
	const StringId  m_attackName;
	int             m_rawDamage;
	int             m_damageType;
	int             m_elementalDamage;
	int             m_elementalDamageType;
	int             m_bleedDamage;
	int             m_critDamage;
	int             m_blockedDamage;
	int             m_finalDamage;
	int             m_hitLocation;
	bool            m_success;
	bool            m_critical;
	bool            m_glancing;
	bool            m_crushing;
	bool            m_strikethrough;
	float           m_strikethroughAmmount;
	bool            m_evadeResult;
	float           m_evadeAmmount;
	bool            m_blockResult;
	int             m_block;
	bool            m_dodge;
	bool            m_parry;
	bool            m_proc;
	Unicode::String m_spamMessage;
	int             m_spamType;
};

//======================================================================

#endif
