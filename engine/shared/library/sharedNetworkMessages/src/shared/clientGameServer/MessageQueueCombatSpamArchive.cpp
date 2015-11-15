//======================================================================
//
// MessageQueueCombatSpamArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCombatSpamArchive.h"

#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedFoundation/NetworkIdArchive.h"

namespace MessageQueueCombatSpamArchiveNamespace
{
	enum DataType
	{
		dt_attackDataWeaponObject,
		dt_attackDataWeaponName,
		dt_messageData
	};
}

using namespace MessageQueueCombatSpamArchiveNamespace;


//======================================================================

MessageQueue::Data * MessageQueueCombatSpamArchive::get (Archive::ReadIterator & source)
{
	uint8       dataType;
	NetworkId   attacker;
	Vector      attackerPosition_w;
	NetworkId   defender;
	Vector      defenderPosition_w;
	NetworkId   weapon;
	StringId    weaponName;
	NetworkId   armor;
	StringId    attackName;
	int         rawDamage = 0;
	int         damageType = 0;
	int         elementalDamage = 0;
	int         elementalDamageType = 0;
	int         bleedDamage = 0;
	int         critDamage = 0;
	int         blockedDamage = 0;
	int         finalDamage = 0;
	int         hitLocation = 0;
	bool        success= false;
	bool		critical= false;
	bool		glancing= false;
	bool        crushing= false;
	bool        strikethrough= false;
	float       strikethroughAmmount= 0.0f;
	bool        evadeResult= false;
	float       evadeAmmount= 0.0f;
	bool        blockResult= false;
	int         block= 0;
	bool        dodge= false;
	bool        parry= false;
	bool		proc= false;
	Unicode::String spamMessage;
	int         spamType;

	Archive::get(source, dataType);
	Archive::get(source, attacker);
	Archive::get(source, attackerPosition_w);
	Archive::get(source, defender);
	Archive::get(source, defenderPosition_w);
	if (dataType == dt_attackDataWeaponObject || dataType == dt_attackDataWeaponName)
	{
		if (dataType == dt_attackDataWeaponObject)
			Archive::get(source, weapon);
		else
			Archive::get(source, weaponName);
		Archive::get(source, attackName);
		Archive::get(source, success);
		if (success)
		{
			Archive::get(source, armor);
			Archive::get(source, rawDamage);
			Archive::get(source, damageType);
			Archive::get(source, elementalDamage);
			Archive::get(source, elementalDamageType);
			Archive::get(source, bleedDamage);
			Archive::get(source, critDamage);
			Archive::get(source, blockedDamage);
			Archive::get(source, finalDamage);
			Archive::get(source, hitLocation);
			Archive::get(source, crushing);
			Archive::get(source, strikethrough);
			Archive::get(source, strikethroughAmmount);
			Archive::get(source, evadeResult);
			Archive::get(source, evadeAmmount);
			Archive::get(source, blockResult);
			Archive::get(source, block);
		}
		else
		{
			Archive::get(source, dodge);
			Archive::get(source, parry);
		}
	}
	else
	{
		Archive::get(source, spamMessage);
	}

	Archive::get(source, critical);
	Archive::get(source, glancing);
	Archive::get(source, proc);
	Archive::get(source, spamType);

	if (dataType == dt_attackDataWeaponObject)
		return new MessageQueueCombatSpam(attacker, attackerPosition_w, defender, defenderPosition_w, weapon, armor, attackName, rawDamage, damageType, elementalDamage, elementalDamageType, bleedDamage, critDamage, blockedDamage, finalDamage, hitLocation, success, critical, glancing, crushing, strikethrough, strikethroughAmmount, evadeResult, evadeAmmount, blockResult, block, dodge, parry, proc, spamType);
	else if (dataType == dt_attackDataWeaponName)
		return new MessageQueueCombatSpam(attacker, attackerPosition_w, defender, defenderPosition_w, weaponName, armor, attackName, rawDamage, damageType, elementalDamage, elementalDamageType, bleedDamage, critDamage, blockedDamage, finalDamage, hitLocation, success, critical, glancing, crushing, strikethrough, strikethroughAmmount, evadeResult, evadeAmmount, blockResult, block, dodge, parry, proc, spamType);
	else
		return new MessageQueueCombatSpam(attacker, attackerPosition_w, defender, defenderPosition_w, spamMessage, critical, glancing, proc, spamType);
}

//----------------------------------------------------------------------

void MessageQueueCombatSpamArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueCombatSpam * const msg = dynamic_cast<const MessageQueueCombatSpam  *>(source);
	NOT_NULL (msg);

	if (msg)
	{
		uint8 dataType;
		if (msg->m_spamMessage.empty())
		{
			if (msg->m_weapon != NetworkId::cms_invalid)
				dataType = dt_attackDataWeaponObject;
			else
				dataType = dt_attackDataWeaponName;
		}
		else
			dataType = dt_messageData;
		Archive::put(target, dataType);
		Archive::put(target, msg->m_attacker);
		Archive::put(target, msg->m_attackerPosition_w);
		Archive::put(target, msg->m_defender);
		Archive::put(target, msg->m_defenderPosition_w);
		if (msg->m_spamMessage.empty())
		{
			if (msg->m_weapon != NetworkId::cms_invalid)
				Archive::put(target, msg->m_weapon);
			else
				Archive::put(target, msg->m_weaponName);
			Archive::put(target, msg->m_attackName);
			Archive::put(target, msg->m_success);
			if (msg->m_success)
			{
				Archive::put(target, msg->m_armor);
				Archive::put(target, msg->m_rawDamage);
				Archive::put(target, msg->m_damageType);
				Archive::put(target, msg->m_elementalDamage);
				Archive::put(target, msg->m_elementalDamageType);
				Archive::put(target, msg->m_bleedDamage);
				Archive::put(target, msg->m_critDamage);
				Archive::put(target, msg->m_blockedDamage);
				Archive::put(target, msg->m_finalDamage);
				Archive::put(target, msg->m_hitLocation);
				Archive::put(target, msg->m_crushing);
				Archive::put(target, msg->m_strikethrough);
				Archive::put(target, msg->m_strikethroughAmmount);
				Archive::put(target, msg->m_evadeResult);
				Archive::put(target, msg->m_evadeAmmount);
				Archive::put(target, msg->m_blockResult);
				Archive::put(target, msg->m_block);
			}
			else
			{
				Archive::put(target, msg->m_dodge);
				Archive::put(target, msg->m_parry);
			}
		}
		else
		{
			Archive::put(target, msg->m_spamMessage);
		}
		Archive::put(target, msg->m_critical);
		Archive::put(target, msg->m_glancing);
		Archive::put(target, msg->m_proc);
		Archive::put(target, msg->m_spamType);
	}
}

//======================================================================
